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

#ifndef LINALG_FFTPLANGPU_HPP_INCLUDED
#define LINALG_FFTPLANGPU_HPP_INCLUDED

// An implementation of FFTPlan which uses a GpuFFTPlan.
//
// When executing the plan the data will be copied to the GPU, transformed there
// and copied back to the CPU.

#include <LinAlg/FFTPlan.hpp>
#include <LinAlg/GpuFFTPlan.hpp>
#include <Math/FPTemplateInstances.hpp>

namespace LinAlg {
  template <typename F> class FFTPlanGpu : public FFTPlan<F> {
    const boost::shared_ptr<GpuFFTPlan<F> > plan;
    OpenCL::VectorAccounting& accounting;
    mutable OpenCL::Vector<std::complex<F> > inbuf;
    mutable OpenCL::Vector<std::complex<F> > outbuf;

    void sync (UNUSED const cl::CommandQueue& queue) const {
      //queue.finish ();
    }

  public:
    FFTPlanGpu (const OpenCL::StubPool& pool, const boost::shared_ptr<GpuFFTPlan<F> >& plan, csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward, bool has128BitAlignment, OpenCL::VectorAccounting& accounting) :
      FFTPlan<F> (size, batchCount, inPlace, outOfPlace, forward, backward, has128BitAlignment),
      plan (plan),
      accounting (accounting),
      inbuf (pool, plan->batchSize (), accounting, "fftInBuf"),
      outbuf (pool, plan->batchSize (), accounting, "fftOutBuf")
    {
      ASSERT (plan);

      ASSERT (pool.context () () == plan->context () ());

      ASSERT (plan->outOfPlace ());

      ASSERT (plan->forward () || !this->forward ());
      ASSERT (plan->backward () || !this->backward ());
      ASSERT (plan->size () == this->size ());
      ASSERT (plan->batchCount () == this->batchCount ());
    }

    virtual ~FFTPlanGpu ();

  protected:
    virtual void doExecute (const std::complex<F>* input, std::complex<F>* output, bool doForward, Core::ProfilingDataPtr prof) const {
      const cl::Context& context = plan->context ();
      cl::CommandQueue queue (context, context.getInfo<CL_CONTEXT_DEVICES>()[0], 0);

      sync (queue);

      {
        Core::ProfileHandle _p2 (prof, "cin");
        inbuf.write (queue, input);
        sync (queue);
      }

      {
        Core::ProfileHandle _p2 (prof, "calc");
        plan->execute (queue, inbuf, outbuf, doForward);
        sync (queue);
      }

      {
        Core::ProfileHandle _p2 (prof, "cout");
        outbuf.read (queue, output);
        sync (queue);
      }
    }
  };

  template <typename F> class FFTPlanGpuFactorySpecialized : public FFTPlanFactory<F> {
    const OpenCL::StubPool& pool_;
    boost::shared_ptr<GpuFFTPlan<F> > plan_;
    OpenCL::VectorAccounting& accounting_;

  public:
    FFTPlanGpuFactorySpecialized (const OpenCL::StubPool& pool, const boost::shared_ptr<GpuFFTPlan<F> >& plan, OpenCL::VectorAccounting& accounting) : FFTPlanFactory<F> (true, true), pool_ (pool), plan_ (plan), accounting_ (accounting) {
      ASSERT (plan);
    }
    virtual ~FFTPlanGpuFactorySpecialized ();

  protected:
    virtual boost::shared_ptr<FFTPlan<F> > doCreatePlan (csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward, bool has128BitAlignment) const {
      return boost::shared_ptr<FFTPlan<F> > (new FFTPlanGpu<F> (pool_, plan_, size, batchCount, inPlace, outOfPlace, forward, backward, has128BitAlignment, accounting_));
    }
  };

  template <typename F> class FFTPlanGpuFactory : public FFTPlanFactory<F> {
    const OpenCL::StubPool& pool_;
    const cl::Device device_;
    const GpuFFTPlanFactory<F>& factory_;
    OpenCL::VectorAccounting& accounting_;

  public:
    FFTPlanGpuFactory (const OpenCL::StubPool& pool, const cl::Device& device, const GpuFFTPlanFactory<F>& factory, OpenCL::VectorAccounting& accounting) : FFTPlanFactory<F> (factory.supportBidirectionalPlan (), factory.supportNonPOTSizes ()), pool_ (pool), device_ (device), factory_ (factory), accounting_ (accounting) {
    }
    virtual ~FFTPlanGpuFactory ();

  protected:
    virtual boost::shared_ptr<FFTPlan<F> > doCreatePlan (csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward, bool has128BitAlignment) const {
      return boost::shared_ptr<FFTPlan<F> > (new FFTPlanGpu<F> (pool_, factory_.createPlan (pool_, device_, size, batchCount, false, true, forward, backward, accounting_), size, batchCount, inPlace, outOfPlace, forward, backward, has128BitAlignment, accounting_));
    }
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, FFTPlanGpu)
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, FFTPlanGpuFactorySpecialized)
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, FFTPlanGpuFactory)
}

#endif // !LINALG_FFTPLANGPU_HPP_INCLUDED
