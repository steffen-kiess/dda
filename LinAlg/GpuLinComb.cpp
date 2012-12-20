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

#include "GpuLinComb.hpp"

#include "GpuLinComb.stub.hpp"

namespace LinAlg {
  template <typename F> GpuLinComb<F>::GpuLinComb (const OpenCL::StubPool& pool, const cl::CommandQueue& queue, OpenCL::VectorAccounting& accounting, Core::ProfilingDataPtr prof) :
    computeUnits (queue.getInfo<CL_QUEUE_DEVICE> ().getInfo<CL_DEVICE_MAX_COMPUTE_UNITS> ()),
    reduce1WgCount (4 * computeUnits),
    reduce1WgSize (128),
    reduce2WgSize (128),
    linCombWgCount (4 * computeUnits),
    linCombWgSize (32),
    temp (pool, reduce1WgCount * 3, accounting, "gpuLinCombTemp")
  {
    pool.set (stub, prof);
  }

  template <typename F> void GpuLinComb<F>::reduce (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, const OpenCL::Pointer<F>& squaredNormOut, const OpenCL::Pointer<std::complex<F> >& selfVecProdConjOut) {
    ASSERT (squaredNormOut || selfVecProdConjOut);

    if (squaredNormOut) {
      if (selfVecProdConjOut) {
        stub->reduceSqnVecOne<F> (queue, reduce1WgCount * reduce1WgSize, reduce1WgSize,
                                  input.size (), temp, input);
        stub->reduceSqnVecOne2<F> (queue, reduce2WgSize, reduce2WgSize,
                                   reduce1WgCount, temp,
                                   squaredNormOut.mem (), squaredNormOut.offset (),
                                   selfVecProdConjOut.mem (), selfVecProdConjOut.offset ());
      } else {
        stub->reduceSqnOne<F> (queue, reduce1WgCount * reduce1WgSize, reduce1WgSize,
                               input.size (), temp, input);
        stub->reduceSqnOne2<F> (queue, reduce2WgSize, reduce2WgSize,
                                reduce1WgCount, temp,
                                squaredNormOut.mem (), squaredNormOut.offset ());
      }
    } else {
      if (selfVecProdConjOut) {
        stub->reduceVecOne<F> (queue, reduce1WgCount * reduce1WgSize, reduce1WgSize,
                               input.size (), temp, input);
        stub->reduceVecOne2<F> (queue, reduce2WgSize, reduce2WgSize,
                                reduce1WgCount, temp,
                                selfVecProdConjOut.mem (), selfVecProdConjOut.offset ());
      } else {
        ABORT ();
      }
    }
  }

  template <typename F> void GpuLinComb<F>::linComb (const cl::CommandQueue& queue,
                                                     const OpenCL::Vector<std::complex<F> >& input1,
                                                     OpenCL::Vector<std::complex<F> >& output) {
    ASSERT (input1.size () == output.size ());

    stub->linCombOne<std::complex<F> > (queue, linCombWgCount * linCombWgSize, linCombWgSize,
                                        output.size (), output,
                                        input1);
  }

  template <typename F> void GpuLinComb<F>::linComb (const cl::CommandQueue& queue,
                                                     const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                                                     OpenCL::Vector<std::complex<F> >& output) {
    ASSERT (input1.size () == output.size ());

    stub->linCombCp<std::complex<F> > (queue, linCombWgCount * linCombWgSize, linCombWgSize,
                                       output.size (), output,
                                       input1, scale1.mem (), scale1.offset ());
  }

  template <typename F> void GpuLinComb<F>::linComb (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input1, UNUSED MinusOne_t minusOne, const OpenCL::Vector<std::complex<F> >& input2, OpenCL::Vector<std::complex<F> >& output) {
    ASSERT (input1.size () == output.size ());
    ASSERT (input2.size () == output.size ());
    stub->linCombNegOne<std::complex<F> > (queue, linCombWgCount * linCombWgSize, linCombWgSize, output.size (), output, input1, input2);
  }

  template <typename F> void GpuLinComb<F>::linComb (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1, const OpenCL::Vector<std::complex<F> >& input2, OpenCL::Vector<std::complex<F> >& output) {
    ASSERT (input1.size () == output.size ());
    ASSERT (input2.size () == output.size ());
    ASSERT (scale1);
    stub->linCombCpOne<std::complex<F> > (queue, linCombWgCount * linCombWgSize, linCombWgSize,
                                          output.size (), output,
                                          input1, scale1.mem (), scale1.offset (),
                                          input2);
  }
  template <typename F> void GpuLinComb<F>::linComb (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const F>& scale1, const OpenCL::Vector<std::complex<F> >& input2, OpenCL::Vector<std::complex<F> >& output) {
    ASSERT (input1.size () == output.size ());
    ASSERT (input2.size () == output.size ());
    ASSERT (scale1);
    stub->linCombRpOne<std::complex<F> > (queue, linCombWgCount * linCombWgSize, linCombWgSize,
                                          output.size (), output,
                                          input1, scale1.mem (), scale1.offset (),
                                          input2);
  }


  template <typename F> void GpuLinComb<F>::linComb (const cl::CommandQueue& queue,
                                                     const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const F>& scale1,
                                                     const OpenCL::Vector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                                                     OpenCL::Vector<std::complex<F> >& output) {
    ASSERT (input1.size () == output.size ());
    ASSERT (input2.size () == output.size ());
    ASSERT (scale1);
    ASSERT (scale2);

    stub->linCombRpCp<std::complex<F> > (queue, linCombWgCount * linCombWgSize, linCombWgSize,
                                         output.size (), output,
                                         input1, scale1.mem (), scale1.offset (),
                                         input2, scale2.mem (), scale2.offset ()
                                         );
  }

  template <typename F> void GpuLinComb<F>::linComb (const cl::CommandQueue& queue,
                                                     const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                                                     const OpenCL::Vector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                                                     OpenCL::Vector<std::complex<F> >& output) {
    ASSERT (input1.size () == output.size ());
    ASSERT (input2.size () == output.size ());
    ASSERT (scale1);
    ASSERT (scale2);

    stub->linCombCpCp<std::complex<F> > (queue, linCombWgCount * linCombWgSize, linCombWgSize,
                                         output.size (), output,
                                         input1, scale1.mem (), scale1.offset (),
                                         input2, scale2.mem (), scale2.offset ()
                                         );
  }

  template <typename F> void GpuLinComb<F>::linComb (const cl::CommandQueue& queue,
                                                     const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                                                     const OpenCL::Vector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                                                     const OpenCL::Vector<std::complex<F> >& input3,
                                                     OpenCL::Vector<std::complex<F> >& output) {
    ASSERT (input1.size () == output.size ());
    ASSERT (input2.size () == output.size ());
    ASSERT (input3.size () == output.size ());
    ASSERT (scale1);
    ASSERT (scale2);

    stub->linCombCpCpOne<std::complex<F> > (queue, linCombWgCount * linCombWgSize, linCombWgSize,
                                            output.size (), output,
                                            input1, scale1.mem (), scale1.offset (),
                                            input2, scale2.mem (), scale2.offset (),
                                            input3
                                            );
  }

  template <typename F> void GpuLinComb<F>::linComb (const cl::CommandQueue& queue,
                                                     const OpenCL::Vector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                                                     const OpenCL::Vector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                                                     const OpenCL::Vector<std::complex<F> >& input3, const OpenCL::Pointer<const std::complex<F> >& scale3,
                                                     OpenCL::Vector<std::complex<F> >& output) {
    ASSERT (input1.size () == output.size ());
    ASSERT (input2.size () == output.size ());
    ASSERT (input3.size () == output.size ());
    ASSERT (scale1);
    ASSERT (scale2);
    ASSERT (scale3);

    stub->linCombCpCpCp<std::complex<F> > (queue, linCombWgCount * linCombWgSize, linCombWgSize,
                                           output.size (), output,
                                           input1, scale1.mem (), scale1.offset (),
                                           input2, scale2.mem (), scale2.offset (),
                                           input3, scale3.mem (), scale3.offset ()
                                           );
  }

  CALL_MACRO_FOR_OPENCL_FP_TYPES(CREATE_TEMPLATE_INSTANCE, GpuLinComb)
}
