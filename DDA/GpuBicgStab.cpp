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

#include "GpuBicgStab.hpp"

#include <Core/Time.hpp>

#include <LinAlg/GpuLinComb.hpp>

#include <sstream>
#include <iomanip>

namespace DDA {
  template <typename F> GpuBicgStab<F>::GpuBicgStab (const OpenCL::StubPool& pool, const std::vector<cl::CommandQueue>& queues, const DDAParams<ftype>& ddaParams, GpuMatVec<ftype>& matVec, csize_t maxIter, OpenCL::VectorAccounting& accounting, Core::ProfilingDataPtr prof) : 
    GpuIterativeSolver<F> (pool, queues, ddaParams, matVec, 30000, maxIter, accounting, prof),
    varsVec (pool, 1, accounting, "vars"),
    vars (varsVec.pointer ()),
    tmpVec2_ (pool, queues, g (), accounting, "tmpVec2"),
    tmpVec3_ (pool, queues, g (), accounting, "tmpVec3"),
    tmpVec4_ (pool, queues, g (), accounting, "tmpVec4")
  {
  }
  template <typename F> GpuBicgStab<F>::~GpuBicgStab () {}

  template <typename F> void GpuBicgStab<F>::init (UNUSED std::ostream& log, UNUSED Core::ProfilingDataPtr prof) {
    DipVector<ftype>& rvec = this->rvec ();
    DipVector<ftype>& rtilda = this->tmpVec4 ();

    this->linComb.linComb (this->queues (), rvec, rtilda);
  }

  namespace {
    template <typename F> std::complex<F> vecProd (const std::vector<cl::CommandQueue>& queues, const DipVector<F>& v1, const DipVector<F>& v2) {
      ASSERT (v1.vectorCount () == v2.vectorCount ());

      std::complex<F> sum = 0;

      for (size_t j = 0; j < v1.vectorCount (); j++) {
        ASSERT (v1[j].size () == v2[j].size ());
        std::vector<std::complex<F> > i1 (v1[j].size () ());
        std::vector<std::complex<F> > i2 (v2[j].size () ());
        v1[j].read (queues[j], i1);
        v2[j].read (queues[j], i2);
      
        for (size_t i = 0; i < v1[j].size (); i++)
          sum += i1[i] * std::conj (i2[i]);
      }

      return sum;
    }
  }

  template <typename F> F GpuBicgStab<F>::iteration (csize_t nr, UNUSED std::ostream& log, bool profilingRun, Core::ProfilingDataPtr prof) {
    const std::vector<cl::CommandQueue>& queues = this->queues ();
    const cl::CommandQueue& queue = queues[0];

    DipVector<ftype>& rvec = this->rvec ();
    DipVector<ftype>& Avecbuffer = this->Avecbuffer ();
    DipVector<ftype>& xvec = this->xvec ();

    DipVector<ftype>& pvec = this->tmpVec1 ();
    DipVector<ftype>& v = this->tmpVec2 ();
    DipVector<ftype>& s = this->tmpVec3 ();
    DipVector<ftype>& rtilda = this->tmpVec4 ();

    (vars + &Vars::ro_new).write (queue, vecProd (queues, rvec, rtilda));
    // Use higher precision to avoid underflow / overflow
    ftype dtmp = static_cast<ftype> (std::abs<ldouble> ((vars + &Vars::ro_new).read (queue)) / this->inprodR ());
    ASSERT (dtmp >= 1e-16 || profilingRun);
    if (nr == 0) {
      this->linComb.linComb (queues, rvec, pvec);
    } else {
      // Use higher precision to avoid underflow / overflow
      cldouble ro_new_alpha = (vars + &Vars::ro_new).read (queue) * (vars + &Vars::alpha).read (queue);
      cldouble ro_old_omega = (vars + &Vars::ro_old).read (queue) * (vars + &Vars::omega).read (queue);
      ASSERT (std::abs (ro_old_omega) / std::abs (ro_new_alpha) >= 10e-10 || profilingRun);
      (vars + &Vars::beta).write (queue, static_cast<ctype> (ro_new_alpha / ro_old_omega));
      (vars + &Vars::mBetaOmega).write (queue, -(vars + &Vars::beta).read (queue) * (vars + &Vars::omega).read (queue));
      this->linComb.linComb (queues, pvec, vars + &Vars::beta, v, vars + &Vars::mBetaOmega, rvec, pvec);
    }
    this->matVec ().apply (queues, pvec, v, false, prof);
    // Use higher precision to avoid underflow / overflow
    cldouble ro_new = (vars + &Vars::ro_new).read (queue);
    cldouble vRtilda = vecProd (queues, v, rtilda);
    (vars + &Vars::alpha).write (queue, static_cast<ctype> (ro_new / vRtilda));
    (vars + &Vars::mAlpha).write (queue, -(vars + &Vars::alpha).read (queue));
    this->linComb.linComb (queues, v, vars + &Vars::mAlpha, rvec, s);
    this->linComb.reduce (queues, s, vars + &Vars::norm);
    ftype inprodRplus1 = (vars + &Vars::norm).read (queue);
    if (inprodRplus1 < this->epsB && !profilingRun) {
      this->linComb.linComb (queues, pvec, vars + &Vars::alpha, xvec, xvec);
    } else {
      this->matVec ().apply (queues, s, Avecbuffer, false, prof);
      this->linComb.reduce (queues, Avecbuffer, vars + &Vars::denumOmega);
      // Use higher precision to avoid underflow / overflow
      //(vars + &Vars::omega).write (queue, vecProd (queues, s, Avecbuffer) / (vars + &Vars::denumOmega).read (queue));
      (vars + &Vars::omega).write (queue, static_cast<ctype> (static_cast<cldouble> (vecProd (queues, s, Avecbuffer)) / static_cast<ldouble> ((vars + &Vars::denumOmega).read (queue))));
      this->linComb.linComb (queues, pvec, vars + &Vars::alpha, s, vars + &Vars::omega, xvec, xvec);
      (vars + &Vars::mOmega).write (queue, -(vars + &Vars::omega).read (queue));
      this->linComb.linComb (queues, Avecbuffer, vars + &Vars::mOmega, s, rvec);
      this->linComb.reduce (queues, rvec, vars + &Vars::norm);
      inprodRplus1 = (vars + &Vars::norm).read (queue);
      (vars + &Vars::ro_old).write (queue, (vars + &Vars::ro_new).read (queue));
    }
    return inprodRplus1;
  }


  CALL_MACRO_FOR_OPENCL_FP_TYPES(CREATE_TEMPLATE_INSTANCE, GpuBicgStab)
}
