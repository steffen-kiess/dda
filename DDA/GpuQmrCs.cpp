/*
 * Copyright (c) 2010-2012 Steffen Kieß
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "GpuQmrCs.hpp"

#include "GpuQmrCs.stub.hpp"

#include <Core/Time.hpp>

#include <DDA/Debug.hpp>

#include <sstream>
#include <iomanip>

namespace DDA {
  //#define INFO(x) Debug::info (#x, this->queues (), x)
#define INFO(x) do { } while (0)

  template <typename F> GpuQmrCs<F>::GpuQmrCs (const OpenCL::StubPool& pool, const std::vector<cl::CommandQueue>& queues, const DDAParams<ftype>& ddaParams, GpuMatVec<ftype>& matVec, csize_t maxIter, OpenCL::VectorAccounting& accounting) : 
    GpuIterativeSolver<F> (pool, queues, ddaParams, matVec, 50000, maxIter, accounting),
    varsVec (pool, 1, accounting, "vars"),
    vars (varsVec.pointer ()),
    tmpVec2_ (pool, queues, g (), accounting, "tmpVec2"),
    tmpVec3_ (pool, queues, g (), accounting, "tmpVec3"),
    tmpVec4_ (pool, queues, g (), accounting, "tmpVec4")
  {
    this->Avecbuffer ().setToZero (queues);
    this->rvec ().setToZero (queues);
    this->xvec ().setToZero (queues);
    this->tmpVec1 ().setToZero (queues);
    tmpVec2 ().setToZero (queues);
    tmpVec3 ().setToZero (queues);
    tmpVec4 ().setToZero (queues);
  }
  template <typename F> GpuQmrCs<F>::~GpuQmrCs () {}

  namespace {
    template <typename F> std::complex<F> vecProdConj (const std::vector<cl::CommandQueue>& queues, const DipVector<F>& v1, const DipVector<F>& v2) {
      ASSERT (v1.vectorCount () == v2.vectorCount ());

      std::complex<F> sum = 0;

      for (size_t j = 0; j < v1.vectorCount (); j++) {
        ASSERT (v1[j].size () == v2[j].size ());
        std::vector<std::complex<F> > i1 (v1[j].size () ());
        std::vector<std::complex<F> > i2 (v2[j].size () ());
        v1[j].read (queues[j], i1);
        v2[j].read (queues[j], i2);
      
        for (size_t i = 0; i < v1[j].size (); i++)
          sum += i1[i] * i2[i];
      }

      return sum;
    }
  }

  template <typename F> void GpuQmrCs<F>::init (UNUSED std::ostream& log, UNUSED Core::ProfilingDataPtr prof) {
    const cl::CommandQueue& queue = this->queues ()[0];
    DipVector<ftype>& v = this->tmpVec1 ();

    //ctype rvec2 = vecProdConj (this->rvec (), this->rvec ());
    this->linComb.reduce (this->queues (), this->rvec (), OpenCL::PointerNull, (vars + &Vars::rvec2));

    (vars + &Vars::omega_old).write (queue, 0);
    (vars + &Vars::beta).write (queue, std::sqrt ((vars + &Vars::rvec2).read (queue)));
    (vars + &Vars::mBeta).write (queue, -(vars + &Vars::beta).read (queue));
    (vars + &Vars::omega_new).write (queue, std::sqrt (this->inprodR ()) / std::abs ((vars + &Vars::beta).read (queue)));
    (vars + &Vars::tautilda).write (queue, (vars + &Vars::omega_new).read (queue) * (vars + &Vars::beta).read (queue));
    (vars + &Vars::c_old).write (queue, 1);
    (vars + &Vars::c_new).write (queue, 1);
    (vars + &Vars::s_old).write (queue, 0);
    (vars + &Vars::s_new).write (queue, 0);
    (vars + &Vars::betaInv).write (queue, Const::one / (vars + &Vars::beta).read (queue));

    //linComb (this->rvec (), Const::one / vars.beta, v);
    this->linComb.linComb (this->queues (), this->rvec (), vars + &Vars::betaInv, v);
    INFO (this->rvec ());
    INFO (vars + &Vars::betaInv);
    INFO (v);
  }

  template <typename F> F GpuQmrCs<F>::iteration (csize_t nr, UNUSED std::ostream& log, UNUSED bool profilingRun, Core::ProfilingDataPtr prof) {
    const std::vector<cl::CommandQueue>& queues = this->queues ();
    const cl::CommandQueue& queue = queues[0];
    INFO ("");

    DipVector<ftype>& rvec = this->rvec ();
    DipVector<ftype>& Avecbuffer = this->Avecbuffer ();
    DipVector<ftype>& xvec = this->xvec ();

    DipVector<ftype>& v = this->tmpVec1 ();
    DipVector<ftype>& vtilda = this->tmpVec2 ();
    DipVector<ftype>& p_old = this->tmpVec3 ();
    DipVector<ftype>& p_new = this->tmpVec4 ();

    ftype rtmp1 = norm ((vars + &Vars::beta).read (queue)) * this->residScale;
    if (nr == 0)
      ASSERT (!(rtmp1 > 1e+38f) || profilingRun); // Allow very low beta values (seen e.g. when using --load-start-dip-pol)
    else
      ASSERT (!(rtmp1 < 1e-10f || rtmp1 > 1e+38f) || profilingRun);

    INFO (rtmp1);

    {
      Core::ProfileHandle _p1 (prof, "matvec");
      this->matVec ().apply (queues, v, Avecbuffer, false, prof);
    }

    ctype alpha = vecProdConj (queues, v, Avecbuffer);

    (vars + &Vars::mAlpha).write (queue, -alpha);

    INFO (vtilda);

    if (nr == 0)
      this->linComb.linComb (queues, v, vars + &Vars::mAlpha, Avecbuffer, vtilda);
    else {
      this->linComb.linComb (queues, vtilda, vars + &Vars::mBeta, v, vars + &Vars::mAlpha, Avecbuffer, vtilda);
    }

    INFO (vars + &Vars::mAlpha); INFO (vars + &Vars::mBeta); INFO (v); INFO (Avecbuffer); INFO (vtilda);

    ctype ctmp3 = vecProdConj (queues, vtilda, vtilda);
    this->linComb.reduce (queues, vtilda, vars + &Vars::vtildaNorm);

    ctype ctmp1 = (vars + &Vars::omega_old).read (queue) * (vars + &Vars::beta).read (queue);
    ctype ctmp2 = (vars + &Vars::omega_new).read (queue) * alpha;

    ctype theta = conj ((vars + &Vars::s_old).read (queue)) * ctmp1;
    ctype eta = (vars + &Vars::c_old).read (queue) * (vars + &Vars::c_new).read (queue) * ctmp1 + conj ((vars + &Vars::s_new).read (queue)) * ctmp2;
    ctype zetatilda = (vars + &Vars::c_new).read (queue) * ctmp2 - (vars + &Vars::c_old).read (queue) * (vars + &Vars::s_new).read (queue) * ctmp1;
    (vars + &Vars::beta).write (queue, std::sqrt (ctmp3));
    (vars + &Vars::mBeta).write (queue, -(vars + &Vars::beta).read (queue));
    (vars + &Vars::omega_old).write (queue, (vars + &Vars::omega_new).read (queue));
    (vars + &Vars::omega_new).write (queue, std::sqrt ((vars + &Vars::vtildaNorm).read (queue)) / abs ((vars + &Vars::beta).read (queue)));
    ftype zetaabs = std::sqrt (norm (zetatilda) + (vars + &Vars::vtildaNorm).read (queue));

    ftype rtmp3 = std::sqrt (norm (zetatilda));
    ctype zeta;
    if (rtmp3 < 1e-40f)
      zeta = zetaabs;
    else
      zeta = zetaabs / rtmp3 * zetatilda;
    (vars + &Vars::c_old).write (queue, (vars + &Vars::c_new).read (queue));
    (vars + &Vars::c_new).write (queue, rtmp3 / zetaabs);
    (vars + &Vars::s_old).write (queue, (vars + &Vars::s_new).read (queue));
    (vars + &Vars::s_new).write (queue, (vars + &Vars::omega_new).read (queue) * ((vars + &Vars::beta).read (queue) / zeta));

    (vars + &Vars::tau).write (queue, (vars + &Vars::c_new).read (queue) * (vars + &Vars::tautilda).read (queue));
    (vars + &Vars::tautilda).write (queue, -(vars + &Vars::s_new).read (queue) * (vars + &Vars::tautilda).read (queue));

    (vars + &Vars::zetaInv).write (queue, Const::one / zeta);
    (vars + &Vars::mEtaZeta).write (queue, -eta / zeta);
    (vars + &Vars::mThetaZeta).write (queue, -theta / zeta);
    (vars + &Vars::betaInv).write (queue, Const::one / (vars + &Vars::beta).read (queue));
    (vars + &Vars::normSNew).write (queue, norm ((vars + &Vars::s_new).read (queue)));
    (vars + &Vars::cNewOmegaNewTautilda).write (queue, ((vars + &Vars::c_new).read (queue) / (vars + &Vars::omega_new).read (queue)) * (vars + &Vars::tautilda).read (queue));

    INFO (vars + &Vars::zetaInv); INFO (vars + &Vars::mEtaZeta); INFO (vars + &Vars::mThetaZeta); INFO (vars + &Vars::betaInv); INFO (vars + &Vars::normSNew); INFO (vars + &Vars::cNewOmegaNewTautilda);

    if (nr == 0) {
      this->linComb.linComb (queues, v, vars + &Vars::zetaInv, p_new);
    } else {
      if (nr == 1)
        this->linComb.linComb (queues, p_new, vars + &Vars::mEtaZeta, v, vars + &Vars::zetaInv, p_old);
      else
        this->linComb.linComb (queues, p_old, vars + &Vars::mThetaZeta, p_new, vars + &Vars::mEtaZeta, v, vars + &Vars::zetaInv, p_old);
      swap (p_old, p_new);
    }

    this->linComb.linComb (queues, p_new, vars + &Vars::tau, xvec, xvec);
    this->linComb.linComb (queues, vtilda, vars + &Vars::betaInv, vtilda);
    swap (v, vtilda);
    this->linComb.linComb (queues, rvec, vars + &Vars::normSNew, v, vars + &Vars::cNewOmegaNewTautilda, rvec);
    INFO (p_new); INFO (p_old); INFO (xvec); INFO (vtilda); INFO (v); INFO (rvec);
    this->linComb.reduce (queues, rvec, vars + &Vars::rvecNorm);
    return (vars + &Vars::rvecNorm).read (queue);
  }


  CALL_MACRO_FOR_OPENCL_FP_TYPES(CREATE_TEMPLATE_INSTANCE, GpuQmrCs)
}
