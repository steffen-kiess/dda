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

#include "GpuCgnr.hpp"

#include <Core/Time.hpp>

#include <LinAlg/GpuLinComb.hpp>

#include <sstream>
#include <iomanip>

namespace DDA {
  template <typename F> GpuCgnr<F>::GpuCgnr (const OpenCL::StubPool& pool, const std::vector<cl::CommandQueue>& queues, const DDAParams<ftype>& ddaParams, GpuMatVec<ftype>& matVec, csize_t maxIter, OpenCL::VectorAccounting& accounting) : 
    GpuIterativeSolver<F> (pool, queues, ddaParams, matVec, 10, maxIter, accounting),
    varsVec (pool, 1, accounting, "vars"),
    vars (varsVec.pointer ())
  {
  }
  template <typename F> GpuCgnr<F>::~GpuCgnr () {}

  template <typename F> void GpuCgnr<F>::init (UNUSED std::ostream& log, UNUSED Core::ProfilingDataPtr prof) {
    (vars + &Vars::ro_old).write (this->queues ()[0], 0);
  }

  template <typename F> F GpuCgnr<F>::iteration (csize_t nr, UNUSED std::ostream& log, UNUSED bool profilingRun, Core::ProfilingDataPtr prof) {
    const std::vector<cl::CommandQueue>& queues = this->queues ();
    const cl::CommandQueue& queue = queues[0];

    DipVector<ftype>& rvec = this->rvec ();
    DipVector<ftype>& Avecbuffer = this->Avecbuffer ();
    DipVector<ftype>& xvec = this->xvec ();
    DipVector<ftype>& pvec = this->tmpVec1 ();

    if (nr == 0) {
      {
        Core::ProfileHandle _p1 (prof, "matvec1");
        this->matVec ().apply (queues, rvec, pvec, true, prof);
      }
      this->linComb.reduce (queues, pvec, vars + &Vars::ro_new);
    } else {
      {
        Core::ProfileHandle _p1 (prof, "matvec1");
        this->matVec ().apply (queues, rvec, Avecbuffer, true, prof);
      }
      this->linComb.reduce (queues, Avecbuffer, vars + &Vars::ro_new);
      (vars + &Vars::beta).write (queue, (vars + &Vars::ro_new).read (queue) / (vars + &Vars::ro_old).read (queue));
      this->linComb.linComb (queues, pvec, vars + &Vars::beta, Avecbuffer, pvec);
    }
    {
      Core::ProfileHandle _p1 (prof, "matvec2");
      this->matVec ().apply (queues, pvec, Avecbuffer, false, prof);
    }
    this->linComb.reduce (queues, Avecbuffer, vars + &Vars::avecNorm);
    (vars + &Vars::alpha).write (queue, (vars + &Vars::ro_new).read (queue) / (vars + &Vars::avecNorm).read (queue));
    (vars + &Vars::alphaNeg).write (queue, -(vars + &Vars::alpha).read (queue));
    this->linComb.linComb (queues, pvec, vars + &Vars::alpha, xvec, xvec);
    this->linComb.linComb (queues, Avecbuffer, vars + &Vars::alphaNeg, rvec, rvec);
    (vars + &Vars::ro_old).write (queue, (vars + &Vars::ro_new).read (queue));
    this->linComb.reduce (queues, rvec, vars + &Vars::rvecNorm);
    return (vars + &Vars::rvecNorm).read (queue);
  }


  CALL_MACRO_FOR_OPENCL_FP_TYPES(CREATE_TEMPLATE_INSTANCE, GpuCgnr)
}
