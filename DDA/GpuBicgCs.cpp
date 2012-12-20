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

#include "GpuBicgCs.hpp"

#include <Core/Time.hpp>

#include <LinAlg/GpuLinComb.hpp>

#include <sstream>
#include <iomanip>

namespace DDA {
  template <typename F> GpuBicgCs<F>::GpuBicgCs (const OpenCL::StubPool& pool, const std::vector<cl::CommandQueue>& queues, const DDAParams<ftype>& ddaParams, GpuMatVec<ftype>& matVec, csize_t maxIter, OpenCL::VectorAccounting& accounting) : 
    GpuIterativeSolver<F> (pool, queues, ddaParams, matVec, 50000, maxIter, accounting),
    varsVec (pool, 1, accounting, "vars"),
    vars (varsVec.pointer ())
  {
  }
  template <typename F> GpuBicgCs<F>::~GpuBicgCs () {}

  template <typename F> void GpuBicgCs<F>::init (UNUSED std::ostream& log, UNUSED Core::ProfilingDataPtr prof) {
  }

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

  template <typename F> F GpuBicgCs<F>::iteration (csize_t nr, UNUSED std::ostream& log, UNUSED bool profilingRun, Core::ProfilingDataPtr prof) {
    const std::vector<cl::CommandQueue>& queues = this->queues ();
    const cl::CommandQueue& queue = queues[0];

    DipVector<ftype>& rvec = this->rvec ();
    DipVector<ftype>& Avecbuffer = this->Avecbuffer ();
    DipVector<ftype>& xvec = this->xvec ();
    DipVector<ftype>& pvec = this->tmpVec1 ();

    (vars + &Vars::ro_new).write (queue, vecProdConj (queues, rvec, rvec));
    (vars + &Vars::abs_ro_new).write (queue, std::abs ((vars + &Vars::ro_new).read (queue)));
    ftype dtmp = (vars + &Vars::abs_ro_new).read (queue) / this->inprodR ();
    ASSERT (!(dtmp < 1e-10 || dtmp > 1e+10) || profilingRun);
    if (nr == 0) {
      this->linComb.linComb (queues, rvec, pvec);
    } else {
      (vars + &Vars::beta).write (queue, (vars + &Vars::ro_new).read (queue) / (vars + &Vars::ro_old).read (queue));
      this->linComb.linComb (queues, pvec, vars + &Vars::beta, rvec, pvec);
    }
    this->matVec ().apply (queues, pvec, Avecbuffer, false, prof);
    ctype mu_k = vecProdConj (queues, pvec, Avecbuffer);
    ftype dtmp2 = std::abs (mu_k) / (vars + &Vars::abs_ro_new).read (queue);
    ASSERT (!(dtmp2 < 10e-10) || profilingRun);
    (vars + &Vars::alpha).write (queue, (vars + &Vars::ro_new).read (queue) / mu_k);
    (vars + &Vars::mAlpha).write (queue, -(vars + &Vars::alpha).read (queue));
    this->linComb.linComb (queues, pvec, vars + &Vars::alpha, xvec, xvec);
    this->linComb.linComb (queues, Avecbuffer, vars + &Vars::mAlpha, rvec, rvec);
    (vars + &Vars::ro_old).write (queue, (vars + &Vars::ro_new).read (queue));
    this->linComb.reduce (queues, rvec, vars + &Vars::rvecNorm);
    return (vars + &Vars::rvecNorm).read (queue);
  }


  CALL_MACRO_FOR_OPENCL_FP_TYPES(CREATE_TEMPLATE_INSTANCE, GpuBicgCs)
}
