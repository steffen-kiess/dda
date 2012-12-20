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

#ifndef LINALG_MULTIGPULINCOMB_HPP_INCLUDED
#define LINALG_MULTIGPULINCOMB_HPP_INCLUDED

// Code for doing linear combinations and vector reductions on vectors
// distributed across several GPUs

#include <OpenCL/MultiGpuVector.hpp>

#include <LinAlg/GpuLinComb.hpp>

namespace LinAlg {
  template <typename F> class MultiGpuLinComb {
    std::vector<boost::shared_ptr<GpuLinComb<F> > > linCombs;

    OpenCL::MultiGpuVector<std::complex<F> > scalesC;
    std::vector<std::complex<F> > scalesCCpu;
    OpenCL::MultiGpuVector<F> scales;
    std::vector<F> scalesCpu;

  public:
    static const typename GpuLinComb<F>::MinusOne_t minusOne;

    MultiGpuLinComb (const OpenCL::StubPool& pool, const std::vector<cl::CommandQueue>& queues, OpenCL::VectorAccounting& accounting, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());

    void reduce (const std::vector<cl::CommandQueue>& queues,
                 const OpenCL::MultiGpuVector<std::complex<F> >& input,
                 const OpenCL::Pointer<F>& squaredNormOut,
                 const OpenCL::Pointer<std::complex<F> >& selfVecProdConjOut = OpenCL::PointerNull);

    void linComb (const std::vector<cl::CommandQueue>& queues,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input1,
                  OpenCL::MultiGpuVector<std::complex<F> >& output);
    void linComb (const std::vector<cl::CommandQueue>& queues,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                  OpenCL::MultiGpuVector<std::complex<F> >& output);


    void linComb (const std::vector<cl::CommandQueue>& queues,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input1, UNUSED typename GpuLinComb<F>::MinusOne_t minusOne,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input2,
                  OpenCL::MultiGpuVector<std::complex<F> >& output);
    void linComb (const std::vector<cl::CommandQueue>& queues,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input2,
                  OpenCL::MultiGpuVector<std::complex<F> >& output);
    void linComb (const std::vector<cl::CommandQueue>& queues,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const F>& scale1,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input2,
                  OpenCL::MultiGpuVector<std::complex<F> >& output);

    void linComb (const std::vector<cl::CommandQueue>& queues,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const F>& scale1,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                  OpenCL::MultiGpuVector<std::complex<F> >& output);
    void linComb (const std::vector<cl::CommandQueue>& queues,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                  OpenCL::MultiGpuVector<std::complex<F> >& output);

    void linComb (const std::vector<cl::CommandQueue>& queues,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input3,
                  OpenCL::MultiGpuVector<std::complex<F> >& output);

    void linComb (const std::vector<cl::CommandQueue>& queues,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                  const OpenCL::MultiGpuVector<std::complex<F> >& input3, const OpenCL::Pointer<const std::complex<F> >& scale3,
                  OpenCL::MultiGpuVector<std::complex<F> >& output);
  };

  CALL_MACRO_FOR_OPENCL_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, MultiGpuLinComb)
}

#endif // !LINALG_MULTIGPULINCOMB_HPP_INCLUDED
