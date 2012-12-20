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

#include "MultiGpuLinComb.hpp"

namespace LinAlg {
  namespace {
    std::vector<size_t> getSizes (csize_t s, csize_t v) {
      std::vector<size_t> ret (s ());
      for (size_t i = 0; i < s; i++)
        ret[i] = v ();
      return ret;
    }

    const int scaleCount = 3;
  }

  template <typename F> MultiGpuLinComb<F>::MultiGpuLinComb (const OpenCL::StubPool& pool, const std::vector<cl::CommandQueue>& queues, OpenCL::VectorAccounting& accounting, UNUSED Core::ProfilingDataPtr prof) :
    linCombs (queues.size ()),
    scalesC (pool, queues, getSizes (linCombs.size (), scaleCount), accounting, "multiGpuLinCombScalesC"),
    scalesCCpu (scaleCount),
    scales (pool, queues, getSizes (linCombs.size (), scaleCount), accounting, "multiGpuLinCombScales"),
    scalesCpu (scaleCount)
  {
    for (size_t i = 0; i < linCombs.size (); i++)
      linCombs[i].reset (new GpuLinComb<F> (pool, queues[i], accounting, prof));
  }

  template <typename F> void MultiGpuLinComb<F>::reduce (const std::vector<cl::CommandQueue>& queues,
                                                         const OpenCL::MultiGpuVector<std::complex<F> >& input,
                                                         const OpenCL::Pointer<F>& squaredNormOut,
                                                         const OpenCL::Pointer<std::complex<F> >& selfVecProdConjOut) {
    ASSERT (queues.size () == linCombs.size ());
    ASSERT (input.vectorCount () == linCombs.size ());
    if (linCombs.size () == 1) {
      linCombs[0]->reduce (queues[0], input[0], squaredNormOut, selfVecProdConjOut);
      return;
    }
    for (size_t i = 0; i < linCombs.size (); i++)
      linCombs[i]->reduce (queues[i], input[i], squaredNormOut ? scales[i].pointer (0) : OpenCL::PointerNull, selfVecProdConjOut ? scalesC[i].pointer (0) : OpenCL::PointerNull);
    if (squaredNormOut) {
      F sqn = 0;
      for (size_t i = 0; i < linCombs.size (); i++)
        sqn += scales[i].pointer (0).read (queues[i]);
      squaredNormOut.write (queues[0], sqn);
    }
    if (selfVecProdConjOut) {
      std::complex<F> vec = 0;
      for (size_t i = 0; i < linCombs.size (); i++)
        vec += scalesC[i].pointer (0).read (queues[i]);
      selfVecProdConjOut.write (queues[0], vec);
    }
  }

  template <typename F> void MultiGpuLinComb<F>::linComb (const std::vector<cl::CommandQueue>& queues,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input1,
                                                          OpenCL::MultiGpuVector<std::complex<F> >& output) {
    ASSERT (queues.size () == linCombs.size ());
    ASSERT (input1.vectorCount () == linCombs.size ());
    ASSERT (output.vectorCount () == linCombs.size ());
    for (size_t i = 0; i < linCombs.size (); i++) {
      ASSERT (queues[i].getInfo<CL_QUEUE_DEVICE> () () == scales.device (i) ());
      linCombs[i]->linComb (queues[i], input1[i], output[i]);
    }
  }
  template <typename F> void MultiGpuLinComb<F>::linComb (const std::vector<cl::CommandQueue>& queues,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                                                          OpenCL::MultiGpuVector<std::complex<F> >& output) {
    ASSERT (queues.size () == linCombs.size ());
    ASSERT (input1.vectorCount () == linCombs.size ());
    ASSERT (output.vectorCount () == linCombs.size ());

    for (size_t i = 0; i < linCombs.size (); i++)
      ASSERT (queues[i].getInfo<CL_QUEUE_DEVICE> () () == scales.device (i) ());

    if (linCombs.size () > 1) {
      scalesCCpu[0] = scale1.read (queues[0]);
      scalesC.writeCopies (queues, scalesCCpu);
    }

    for (size_t i = 0; i < linCombs.size (); i++)
      linCombs[i]->linComb (queues[i], input1[i], linCombs.size () > 1 ? scalesC[i].pointer (0) : scale1, output[i]);
  }


  template <typename F> void MultiGpuLinComb<F>::linComb (const std::vector<cl::CommandQueue>& queues,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input1, UNUSED typename GpuLinComb<F>::MinusOne_t minusOne,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input2,
                                                          OpenCL::MultiGpuVector<std::complex<F> >& output) {
    ASSERT (queues.size () == linCombs.size ());
    ASSERT (input1.vectorCount () == linCombs.size ());
    ASSERT (input2.vectorCount () == linCombs.size ());
    ASSERT (output.vectorCount () == linCombs.size ());

    for (size_t i = 0; i < linCombs.size (); i++)
      ASSERT (queues[i].getInfo<CL_QUEUE_DEVICE> () () == scales.device (i) ());

    for (size_t i = 0; i < linCombs.size (); i++)
      linCombs[i]->linComb (queues[i], input1[i], GpuLinComb<F>::minusOne, input2[i], output[i]);
  }
  template <typename F> void MultiGpuLinComb<F>::linComb (const std::vector<cl::CommandQueue>& queues,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input2,
                                                          OpenCL::MultiGpuVector<std::complex<F> >& output) {
    ASSERT (queues.size () == linCombs.size ());
    ASSERT (input1.vectorCount () == linCombs.size ());
    ASSERT (input2.vectorCount () == linCombs.size ());
    ASSERT (output.vectorCount () == linCombs.size ());

    for (size_t i = 0; i < linCombs.size (); i++)
      ASSERT (queues[i].getInfo<CL_QUEUE_DEVICE> () () == scales.device (i) ());

    if (linCombs.size () > 1) {
      scalesCCpu[0] = scale1.read (queues[0]);
      scalesC.writeCopies (queues, scalesCCpu);
    }

    for (size_t i = 0; i < linCombs.size (); i++)
      linCombs[i]->linComb (queues[i], input1[i], linCombs.size () > 1 ? scalesC[i].pointer (0) : scale1, input2[i], output[i]);
  }
  template <typename F> void MultiGpuLinComb<F>::linComb (const std::vector<cl::CommandQueue>& queues,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const F>& scale1,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input2,
                                                          OpenCL::MultiGpuVector<std::complex<F> >& output) {
    ASSERT (queues.size () == linCombs.size ());
    ASSERT (input1.vectorCount () == linCombs.size ());
    ASSERT (input2.vectorCount () == linCombs.size ());
    ASSERT (output.vectorCount () == linCombs.size ());

    for (size_t i = 0; i < linCombs.size (); i++)
      ASSERT (queues[i].getInfo<CL_QUEUE_DEVICE> () () == scales.device (i) ());

    if (linCombs.size () > 1) {
      scalesCpu[0] = scale1.read (queues[0]);
      scales.writeCopies (queues, scalesCpu);
    }

    for (size_t i = 0; i < linCombs.size (); i++)
      linCombs[i]->linComb (queues[i], input1[i], linCombs.size () > 1 ? scales[i].pointer (0) : scale1, input2[i], output[i]);
  }

  template <typename F> void MultiGpuLinComb<F>::linComb (const std::vector<cl::CommandQueue>& queues,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const F>& scale1,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                                                          OpenCL::MultiGpuVector<std::complex<F> >& output) {
    ASSERT (queues.size () == linCombs.size ());
    ASSERT (input1.vectorCount () == linCombs.size ());
    ASSERT (input2.vectorCount () == linCombs.size ());
    ASSERT (output.vectorCount () == linCombs.size ());

    for (size_t i = 0; i < linCombs.size (); i++)
      ASSERT (queues[i].getInfo<CL_QUEUE_DEVICE> () () == scales.device (i) ());

    if (linCombs.size () > 1) {
      scalesCpu[0] = scale1.read (queues[0]);
      scalesCCpu[1] = scale2.read (queues[0]);
      scales.writeCopies (queues, scalesCpu);
      scalesC.writeCopies (queues, scalesCCpu);
    }

    for (size_t i = 0; i < linCombs.size (); i++)
      linCombs[i]->linComb (queues[i], input1[i], linCombs.size () > 1 ? scales[i].pointer (0) : scale1, input2[i], linCombs.size () > 1 ? scalesC[i].pointer (1) : scale2, output[i]);
  }
  template <typename F> void MultiGpuLinComb<F>::linComb (const std::vector<cl::CommandQueue>& queues,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                                                          OpenCL::MultiGpuVector<std::complex<F> >& output) {
    ASSERT (queues.size () == linCombs.size ());
    ASSERT (input1.vectorCount () == linCombs.size ());
    ASSERT (input2.vectorCount () == linCombs.size ());
    ASSERT (output.vectorCount () == linCombs.size ());

    for (size_t i = 0; i < linCombs.size (); i++)
      ASSERT (queues[i].getInfo<CL_QUEUE_DEVICE> () () == scales.device (i) ());

    if (linCombs.size () > 1) {
      scalesCCpu[0] = scale1.read (queues[0]);
      scalesCCpu[1] = scale2.read (queues[0]);
      scalesC.writeCopies (queues, scalesCCpu);
    }

    for (size_t i = 0; i < linCombs.size (); i++)
      linCombs[i]->linComb (queues[i], input1[i], linCombs.size () > 1 ? scalesC[i].pointer (0) : scale1, input2[i], linCombs.size () > 1 ? scalesC[i].pointer (1) : scale2, output[i]);
  }

  template <typename F> void MultiGpuLinComb<F>::linComb (const std::vector<cl::CommandQueue>& queues,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input3,
                                                          OpenCL::MultiGpuVector<std::complex<F> >& output) {
    ASSERT (queues.size () == linCombs.size ());
    ASSERT (input1.vectorCount () == linCombs.size ());
    ASSERT (input2.vectorCount () == linCombs.size ());
    ASSERT (input3.vectorCount () == linCombs.size ());
    ASSERT (output.vectorCount () == linCombs.size ());

    for (size_t i = 0; i < linCombs.size (); i++)
      ASSERT (queues[i].getInfo<CL_QUEUE_DEVICE> () () == scales.device (i) ());

    if (linCombs.size () > 1) {
      scalesCCpu[0] = scale1.read (queues[0]);
      scalesCCpu[1] = scale2.read (queues[0]);
      scalesC.writeCopies (queues, scalesCCpu);
    }

    for (size_t i = 0; i < linCombs.size (); i++)
      linCombs[i]->linComb (queues[i], input1[i], linCombs.size () > 1 ? scalesC[i].pointer (0) : scale1, input2[i], linCombs.size () > 1 ? scalesC[i].pointer (1) : scale2, input3[i], output[i]);
  }

  template <typename F> void MultiGpuLinComb<F>::linComb (const std::vector<cl::CommandQueue>& queues,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input1, const OpenCL::Pointer<const std::complex<F> >& scale1,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input2, const OpenCL::Pointer<const std::complex<F> >& scale2,
                                                          const OpenCL::MultiGpuVector<std::complex<F> >& input3, const OpenCL::Pointer<const std::complex<F> >& scale3,
                                                          OpenCL::MultiGpuVector<std::complex<F> >& output) {
    ASSERT (queues.size () == linCombs.size ());
    ASSERT (input1.vectorCount () == linCombs.size ());
    ASSERT (input2.vectorCount () == linCombs.size ());
    ASSERT (input3.vectorCount () == linCombs.size ());
    ASSERT (output.vectorCount () == linCombs.size ());

    for (size_t i = 0; i < linCombs.size (); i++)
      ASSERT (queues[i].getInfo<CL_QUEUE_DEVICE> () () == scales.device (i) ());

    if (linCombs.size () > 1) {
      scalesCCpu[0] = scale1.read (queues[0]);
      scalesCCpu[1] = scale2.read (queues[0]);
      scalesCCpu[2] = scale3.read (queues[0]);
      scalesC.writeCopies (queues, scalesCCpu);
    }

    for (size_t i = 0; i < linCombs.size (); i++)
      linCombs[i]->linComb (queues[i], input1[i], linCombs.size () > 1 ? scalesC[i].pointer (0) : scale1, input2[i], linCombs.size () > 1 ? scalesC[i].pointer (1) : scale2, input3[i], linCombs.size () > 1 ? scalesC[i].pointer (2) : scale3, output[i]);
  }

  CALL_MACRO_FOR_OPENCL_FP_TYPES(CREATE_TEMPLATE_INSTANCE, MultiGpuLinComb)
}
