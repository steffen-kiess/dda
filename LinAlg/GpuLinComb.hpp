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

#ifndef LINALG_GPULINCOMB_HPP_INCLUDED
#define LINALG_GPULINCOMB_HPP_INCLUDED

// Code for doing linear combinations and vector reductions on the GPU

#include <OpenCL/Vector.hpp>
#include <OpenCL/StubPool.hpp>
#include <OpenCL/Pointer.hpp>

#include <Math/FPTemplateInstances.hpp>

#include <complex>

namespace LinAlg {
  template <typename F> class GpuLinComb {
    boost::shared_ptr<class GpuLinCombStub> stub;

    size_t computeUnits;

    size_t reduce1WgCount;
    size_t reduce1WgSize;

    size_t reduce2WgSize;

    size_t linCombWgCount;
    size_t linCombWgSize;

    OpenCL::Vector<F> temp;

  public:
    GpuLinComb (const OpenCL::StubPool& pool, const cl::CommandQueue& queue, OpenCL::VectorAccounting& accounting, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());

    struct MinusOne_t {};
    static const MinusOne_t minusOne;

    void reduce (const cl::CommandQueue& queue,
                 const OpenCL::Vector<std::complex<F> >& input,
                 const OpenCL::Pointer<F>& squaredNormOut,
                 const OpenCL::Pointer<std::complex<F> >& selfVecProdConjOut = OpenCL::PointerNull);

    void linComb (const cl::CommandQueue& queue,
                  const OpenCL::Vector<std::complex<F> >& input1,
                  OpenCL::Vector<std::complex<F> >& output);
    void linComb (const cl::CommandQueue& queue,
                  const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                  OpenCL::Vector<std::complex<F> >& output);


    void linComb (const cl::CommandQueue& queue,
                  const OpenCL::Vector<std::complex<F> >& input1, UNUSED MinusOne_t minusOne,
                  const OpenCL::Vector<std::complex<F> >& input2,
                  OpenCL::Vector<std::complex<F> >& output);
    void linComb (const cl::CommandQueue& queue,
                  const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                  const OpenCL::Vector<std::complex<F> >& input2,
                  OpenCL::Vector<std::complex<F> >& output);
    void linComb (const cl::CommandQueue& queue,
                  const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const F>& scale1,
                  const OpenCL::Vector<std::complex<F> >& input2,
                  OpenCL::Vector<std::complex<F> >& output);

    void linComb (const cl::CommandQueue& queue,
                  const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const F>& scale1,
                  const OpenCL::Vector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                  OpenCL::Vector<std::complex<F> >& output);
    void linComb (const cl::CommandQueue& queue,
                  const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                  const OpenCL::Vector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                  OpenCL::Vector<std::complex<F> >& output);

    void linComb (const cl::CommandQueue& queue,
                  const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                  const OpenCL::Vector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                  const OpenCL::Vector<std::complex<F> >& input3,
                  OpenCL::Vector<std::complex<F> >& output);

    void linComb (const cl::CommandQueue& queue,
                  const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                  const OpenCL::Vector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                  const OpenCL::Vector<std::complex<F> >& input3, const OpenCL::Pointer<const std::complex<F> >& scale3,
                  OpenCL::Vector<std::complex<F> >& output);
  };

  CALL_MACRO_FOR_OPENCL_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, GpuLinComb)
}


#endif // !LINALG_GPULINCOMB_HPP_INCLUDED
