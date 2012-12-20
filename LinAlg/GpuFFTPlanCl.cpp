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

#include "GpuFFTPlanCl.hpp"

#include <float.generated/clFFT.h>
#include <double.generated/clFFT.h>

namespace LinAlg {
  namespace {
    const bool supportNonZeroOffsets = false;

    template <class F> class GpuFFTPlanCl : public GpuFFTPlan<F> {
      void* plan;
      mutable OpenCL::Vector<std::complex<F> > tmp;

    public:
      GpuFFTPlanCl (const OpenCL::StubPool& pool, const cl::Device& device, csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward, OpenCL::VectorAccounting& accounting);

      virtual ~GpuFFTPlanCl ();

    protected:
      virtual void doExecute (const cl::CommandQueue& queue, const cl::Buffer& input, csize_t inputOffset, const cl::Buffer& output, csize_t outputOffset, bool doForward) const;
    };

    template <typename F> class GpuFFTPlanClFactory : public GpuFFTPlanFactory<F> {
    public:
      GpuFFTPlanClFactory () : GpuFFTPlanFactory<F> (true, false) {
      }

    protected:
      virtual boost::shared_ptr<GpuFFTPlan<F> > doCreatePlan (const OpenCL::StubPool& pool, const cl::Device& device, csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward, OpenCL::VectorAccounting& accounting) const {
        return boost::shared_ptr<GpuFFTPlan<F> > (new GpuFFTPlanCl<F> (pool, device, size, batchCount, inPlace, outOfPlace, forward, backward, accounting));
      }
    };


    template <typename F> struct ClFFTOperations;
#define VAL(x) (ClFFTOperations<F>::val_##x ())
#define TY(x) typename ClFFTOperations<F>::x

    template <typename F> GpuFFTPlanCl<F>::GpuFFTPlanCl (const OpenCL::StubPool& pool, const cl::Device& device, csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward, OpenCL::VectorAccounting& accounting) : GpuFFTPlan<F> (pool.context (), device, size, batchCount, inPlace, outOfPlace, forward, backward), tmp (pool, supportNonZeroOffsets ? this->batchSize () : 0, accounting, "GpuFFTPlanCl tmp") {
      TY(Dim3) dim;
      dim.x = Core::checked_cast<unsigned int> (size);
      dim.y = 1;
      dim.z = 1;

      cl_int error = CL_SUCCESS;
      plan = static_cast<void*> (VAL(CreatePlan) (this->context () (), dim, VAL(1D), VAL(InterleavedComplexFormat), &error));
      cl::detail::errHandler (error, VAL(CreatePlanStr));
      ASSERT (plan != NULL);
    }

    template <typename F> GpuFFTPlanCl<F>::~GpuFFTPlanCl () {
      VAL(DestroyPlan) (static_cast<TY(Plan)> (plan));
    }

    template <typename F> void GpuFFTPlanCl<F>::doExecute (const cl::CommandQueue& queue, const cl::Buffer& input, csize_t inputOffset, const cl::Buffer& output, csize_t outputOffset, bool doForward) const {
      if (supportNonZeroOffsets) {
        if (inputOffset != 0 || outputOffset != 0) {
          queue.enqueueCopyBuffer (input, tmp.getDataWritable (), (inputOffset * sizeof (std::complex<F>)) (), 0, (csize_t (sizeof (std::complex<F>)) * this->batchSize ()).value ());
          doExecute (queue, tmp.getDataWritable (), 0, tmp.getDataWritable (), 0, doForward);
          queue.enqueueCopyBuffer (tmp.getData (), output, 0, (outputOffset * sizeof (std::complex<F>)) (), (csize_t (sizeof (std::complex<F>)) * this->batchSize ()).value ());
          return;
        }
      } else {
        ASSERT_MSG (inputOffset == 0, "not implemented");
        ASSERT_MSG (outputOffset == 0, "not implemented");
      }

      if (this->size () == 0)
        return;
      if (this->size () == 1) {
        if (input () != output ())
          queue.enqueueCopyBuffer (input, output, 0, 0, (csize_t (sizeof (std::complex<F>)) * this->batchCount ()).value ());
        return;
      }

      cl::detail::errHandler (VAL(ExecuteInterleaved) (queue (), static_cast<TY(Plan)> (plan), Core::checked_cast<int> (this->batchCount ()), doForward ? VAL(Forward) : VAL(Inverse), input (), output (), 0, NULL, NULL), VAL(ExecuteInterleavedStr));
    }
#undef TY
#undef VAL

    template <> struct ClFFTOperations<float> {
      typedef clFFT_Dim3 Dim3;
      typedef clFFT_Plan Plan;
#define FN(x) static inline __typeof__ (&clFFT_##x) val_##x () { return clFFT_##x; } \
      static inline const char* val_##x##Str () { return #x; }
      FN (CreatePlan)
      FN (DestroyPlan)
      FN (ExecuteInterleaved)
#undef FN
#define VAL(x) static inline __typeof__ (clFFT_##x) val_##x () { return clFFT_##x; }
      VAL (1D)
      VAL (InterleavedComplexFormat)
      VAL (Forward)
      VAL (Inverse)
#undef VAL
    };

    template <> struct ClFFTOperations<double> {
      typedef clFFTdouble_Dim3 Dim3;
      typedef clFFTdouble_Plan Plan;
#define FN(x) static inline __typeof__ (&clFFTdouble_##x) val_##x () { return clFFTdouble_##x; } \
      static inline const char* val_##x##Str () { return #x; }
      FN (CreatePlan)
      FN (DestroyPlan)
      FN (ExecuteInterleaved)
#undef FN
#define VAL(x) static inline __typeof__ (clFFTdouble_##x) val_##x () { return clFFTdouble_##x; }
      VAL (1D)
      VAL (InterleavedComplexFormat)
      VAL (Forward)
      VAL (Inverse)
#undef VAL
    };
  }

  template <typename F> const GpuFFTPlanFactory<F>& getGpuFFTPlanClFactory () {
    static GpuFFTPlanClFactory<F> factory;
    return factory;
  }

  template const GpuFFTPlanFactory<float>& getGpuFFTPlanClFactory ();
  template const GpuFFTPlanFactory<double>& getGpuFFTPlanClFactory ();
}
