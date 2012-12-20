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

#include "MatVecGpu.hpp"

namespace DDA {
  namespace {
    std::vector<cl::CommandQueue> queues (const cl::CommandQueue& queue) {
      std::vector<cl::CommandQueue> ret (1);
      ret[0] = queue;
      return ret;
    }
  }

  template <class F> MatVecGpu<F>::MatVecGpu (const OpenCL::StubPool& pool, const cl::CommandQueue& queue, GpuMatVec<ftype>& matVec, OpenCL::VectorAccounting& accounting) :
    MatVec<ftype> (matVec.g ()),
    pool (pool),
    matVec (matVec),
    argGpu (pool, queues (queue), matVec.g (), accounting, "matVecGpuArg"),
    resultGpu (pool, queues (queue), matVec.g (), accounting, "matVecGpuResult"),
    queue (queue)
  {
    ASSERT (matVec.g ().procs () == 1);
  }
  template <class F> MatVecGpu<F>::~MatVecGpu () {}

  template <class F> void MatVecGpu<F>::setCoupleConstants (const boost::shared_ptr<const CoupleConstants<ftype> >& cc) {
    MatVec<F>::setCoupleConstants (cc);
    matVec.setCoupleConstants (queues (queue), cc);
  }

  template <class F> void MatVecGpu<F>::apply (const std::vector<ctype>& arg, std::vector<ctype>& result, bool conj, Core::ProfilingDataPtr prof) {
    //Core::ProfileHandle* p1;
    //p1 = new Core::ProfileHandle (prof, "setup");
    //cl::CommandQueue queue (pool.context (), pool.context ().getInfo<CL_CONTEXT_DEVICES>()[0], 0);
    //delete p1;

    {
      Core::ProfileHandle _p (prof, "cin");
      argGpu[0].write (queue, arg);
    }

    matVec.apply (queues (queue), argGpu, resultGpu, conj, prof);

    {
      Core::ProfileHandle _p (prof, "cout");
      resultGpu[0].read (queue, result);
    }
  }

  CALL_MACRO_FOR_OPENCL_FP_TYPES(CREATE_TEMPLATE_INSTANCE, MatVecGpu)
}
