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

#ifndef LINALG_GPUFFTPLAN_HPP_INCLUDED
#define LINALG_GPUFFTPLAN_HPP_INCLUDED

// LinAlg::GpuFFTPlan is an abstract class for a 1d-fft plan running on the GPU
//
// LinAlg::GpuFFTPlanFactory is an abstract factory of GpuFFTPlans

#include <Core/Assert.hpp>
#include <Core/CheckedInteger.hpp>

#include <OpenCL/Vector.hpp>
#include <OpenCL/Bindings.hpp>
#include <OpenCL/Util.hpp>

#include <Math/FPTemplateInstances.hpp>

#include <complex>

namespace LinAlg {
  template <typename F> class GpuFFTPlan {
    cl::Context _context;
    cl::Device _device;
    csize_t _size;
    csize_t _batchCount;
    csize_t _batchSize;
    bool _inPlace;
    bool _outOfPlace;
    bool _forward;
    bool _backward;

  public:
    virtual ~GpuFFTPlan ();

  protected:
    GpuFFTPlan (const cl::Context& context, const cl::Device& device, csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward) : _context (context), _device (device), _size (size), _batchCount (batchCount), _batchSize (size * batchCount), _inPlace (inPlace), _outOfPlace (outOfPlace), _forward (forward), _backward (backward) {
      ASSERT (context () != NULL);
      ASSERT (device () != NULL);
      ASSERT (inPlace || outOfPlace);
      ASSERT (forward || backward);
    }

    virtual void doExecute (const cl::CommandQueue& queue, const cl::Buffer& input, csize_t inputOffset, const cl::Buffer& output, csize_t outputOffset, bool doForward) const = 0;

  public:
    cl::Context context () const {
      return _context;
    }

    cl::Device device () const {
      return _device;
    }

    bool inPlace () const {
      return _inPlace;
    }

    bool outOfPlace () const {
      return _outOfPlace;
    }

    bool forward () const {
      return _forward;
    }

    bool backward () const {
      return _backward;
    }

    csize_t size () const {
      return _size;
    }

    csize_t batchCount () const {
      return _batchCount;
    }

    csize_t batchSize () const {
      return _batchSize;
    }

    void execute (const cl::CommandQueue& queue, const cl::Buffer& input, csize_t inputOffset, const cl::Buffer& output, csize_t outputOffset, bool doForward) const {
      ASSERT (inputOffset + batchSize () * sizeof (F) <= input.getInfo<CL_MEM_SIZE> ());
      ASSERT (outputOffset + batchSize () * sizeof (F) <= output.getInfo<CL_MEM_SIZE> ());

      if (input () == output ()) {
        if (inputOffset == outputOffset) {
          ASSERT (inPlace ());
        } else {
          ASSERT (outOfPlace ());
          if (inputOffset > outputOffset)
            ASSERT (inputOffset >= outputOffset + batchSize ());
          else
            ASSERT (outputOffset >= inputOffset + batchSize ());
        }
      } else {
        ASSERT (outOfPlace ());
      }

      if (doForward)
        ASSERT (forward ());
      else
        ASSERT (backward ());

      doExecute (queue, input, inputOffset, output, outputOffset, doForward);
    }
    void execute (const cl::CommandQueue& queue, const cl::Buffer& input, const cl::Buffer& output, bool doForward) const {
      execute (queue, input, 0, output, 0, doForward);
    }

    void fft (const cl::CommandQueue& queue, const cl::Buffer& input, csize_t inputOffset, const cl::Buffer& output, csize_t outputOffset) const {
      execute (queue, input, inputOffset, output, outputOffset, true);
    }
    void fft (const cl::CommandQueue& queue, const cl::Buffer& input, const cl::Buffer& output) const {
      execute (queue, input, output, true);
    }

    void ifft (const cl::CommandQueue& queue, const cl::Buffer& input, csize_t inputOffset, const cl::Buffer& output, csize_t outputOffset) const {
      execute (queue, input, inputOffset, output, outputOffset, false);
    }
    void ifft (const cl::CommandQueue& queue, const cl::Buffer& input, const cl::Buffer& output) const {
      execute (queue, input, output, false);
    }

    void executeOutOfPlace (const cl::CommandQueue& queue, const cl::Buffer& input, csize_t inputOffset, const cl::Buffer& output, csize_t outputOffset, bool doForward) const {
      ASSERT (input () != output () || inputOffset != outputOffset);
      execute (queue, input, inputOffset, output, outputOffset, doForward);
    }
    void executeOutOfPlace (const cl::CommandQueue& queue, const cl::Buffer& input, const cl::Buffer& output, bool doForward) const {
      ASSERT (input () != output ());
      execute (queue, input, output, doForward);
    }

    void fftOutOfPlace (const cl::CommandQueue& queue, const cl::Buffer& input, csize_t inputOffset, const cl::Buffer& output, csize_t outputOffset) const {
      executeOutOfPlace (queue, input, inputOffset, output, outputOffset, true);
    }
    void fftOutOfPlace (const cl::CommandQueue& queue, const cl::Buffer& input, const cl::Buffer& output) const {
      executeOutOfPlace (queue, input, output, true);
    }

    void ifftOutOfPlace (const cl::CommandQueue& queue, const cl::Buffer& input, csize_t inputOffset, const cl::Buffer& output, csize_t outputOffset) const {
      executeOutOfPlace (queue, input, inputOffset, output, outputOffset, false);
    }
    void ifftOutOfPlace (const cl::CommandQueue& queue, const cl::Buffer& input, const cl::Buffer& output) const {
      executeOutOfPlace (queue, input, output, false);
    }

    void executeInPlace (const cl::CommandQueue& queue, const cl::Buffer& inout, csize_t offset, bool doForward) const {
      execute (queue, inout, offset, inout, offset, doForward);
    }
    void executeInPlace (const cl::CommandQueue& queue, const cl::Buffer& inout, bool doForward) const {
      execute (queue, inout, inout, doForward);
    }

    void fftInPlace (const cl::CommandQueue& queue, const cl::Buffer& inout, csize_t offset) const {
      executeInPlace (queue, inout, offset, true);
    }
    void fftInPlace (const cl::CommandQueue& queue, const cl::Buffer& inout) const {
      executeInPlace (queue, inout, true);
    }

    void ifftInPlace (const cl::CommandQueue& queue, const cl::Buffer& inout, csize_t offset) const {
      executeInPlace (queue, inout, offset, false);
    }
    void ifftInPlace (const cl::CommandQueue& queue, const cl::Buffer& inout) const {
      executeInPlace (queue, inout, false);
    }

    void execute (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, csize_t inputOffset, OpenCL::Vector<std::complex<F> >& output, csize_t outputOffset, bool doForward) const {
      ASSERT (csize_t (input.getSize ()) >= batchSize () + inputOffset);
      ASSERT (csize_t (output.getSize ()) >= batchSize () + outputOffset);

      if (&input != &output)
        ASSERT (input.getData ()() != output.getDataWritable ()());

      execute (queue, input.getData (), inputOffset, output.getDataWritable (), outputOffset, doForward);
    }
    void execute (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, OpenCL::Vector<std::complex<F> >& output, bool doForward) const {
      ASSERT (input.getSize () == batchSize ());
      ASSERT (output.getSize () == batchSize ());

      execute (queue, input, 0, output, 0, doForward);
    }

    void fft (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, csize_t inputOffset, OpenCL::Vector<std::complex<F> >& output, csize_t outputOffset) const {
      execute (queue, input, inputOffset, output, outputOffset, true);
    }
    void fft (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, OpenCL::Vector<std::complex<F> >& output) const {
      execute (queue, input, output, true);
    }

    void ifft (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, csize_t inputOffset, OpenCL::Vector<std::complex<F> >& output, csize_t outputOffset) const {
      execute (queue, input, inputOffset, output, outputOffset, false);
    }
    void ifft (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, OpenCL::Vector<std::complex<F> >& output) const {
      execute (queue, input, output, false);
    }

    void executeOutOfPlace (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, csize_t inputOffset, OpenCL::Vector<std::complex<F> >& output, csize_t outputOffset, bool doForward) const {
      ASSERT (&input != &output);
      execute (queue, input, inputOffset, output, outputOffset, doForward);
    }
    void executeOutOfPlace (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, OpenCL::Vector<std::complex<F> >& output, bool doForward) const {
      ASSERT (&input != &output);
      execute (queue, input, output, doForward);
    }

    void fftOutOfPlace (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, csize_t inputOffset, OpenCL::Vector<std::complex<F> >& output, csize_t outputOffset) const {
      executeOutOfPlace (queue, input, inputOffset, output, outputOffset, true);
    }
    void fftOutOfPlace (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, OpenCL::Vector<std::complex<F> >& output) const {
      executeOutOfPlace (queue, input, output, true);
    }

    void ifftOutOfPlace (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, csize_t inputOffset, OpenCL::Vector<std::complex<F> >& output, csize_t outputOffset) const {
      executeOutOfPlace (queue, input, inputOffset, output, outputOffset, false);
    }
    void ifftOutOfPlace (const cl::CommandQueue& queue, const OpenCL::Vector<std::complex<F> >& input, OpenCL::Vector<std::complex<F> >& output) const {
      executeOutOfPlace (queue, input, output, false);
    }

    void executeInPlace (const cl::CommandQueue& queue, OpenCL::Vector<std::complex<F> >& inout, csize_t offset, bool doForward) const {
      execute (queue, inout, offset, inout, offset, doForward);
    }
    void executeInPlace (const cl::CommandQueue& queue, OpenCL::Vector<std::complex<F> >& inout, bool doForward) const {
      execute (queue, inout, inout, doForward);
    }

    void fftInPlace (const cl::CommandQueue& queue, OpenCL::Vector<std::complex<F> >& inout, csize_t offset) const {
      executeInPlace (queue, inout, offset, true);
    }
    void fftInPlace (const cl::CommandQueue& queue, OpenCL::Vector<std::complex<F> >& inout) const {
      executeInPlace (queue, inout, true);
    }

    void ifftInPlace (const cl::CommandQueue& queue, OpenCL::Vector<std::complex<F> >& inout, csize_t offset) const {
      executeInPlace (queue, inout, offset, false);
    }
    void ifftInPlace (const cl::CommandQueue& queue, OpenCL::Vector<std::complex<F> >& inout) const {
      executeInPlace (queue, inout, false);
    }
  };

  template <typename F> class GpuFFTPlanFactory;
  template <typename F> class GpuFFTPlanPair : public GpuFFTPlan<F> {
    friend class GpuFFTPlanFactory<F>;

    boost::shared_ptr<GpuFFTPlan<F> > forwardPlan_;
    boost::shared_ptr<GpuFFTPlan<F> > backwardPlan_;

    GpuFFTPlanPair (const boost::shared_ptr<GpuFFTPlan<F> >& forwardPlan, const boost::shared_ptr<GpuFFTPlan<F> >& backwardPlan, csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace) : GpuFFTPlan<F> (forwardPlan->context (), forwardPlan->device (), size, batchCount, inPlace, outOfPlace, true, true), forwardPlan_ (forwardPlan), backwardPlan_ (backwardPlan) {
      ASSERT (forwardPlan);
      ASSERT (backwardPlan);
      ASSERT (backwardPlan->context () () == this->context () ());
      ASSERT (backwardPlan->device () () == this->device () ());
    }

  public:
    virtual ~GpuFFTPlanPair ();

  protected:
    virtual void doExecute (const cl::CommandQueue& queue, const cl::Buffer& input, csize_t inputOffset, const cl::Buffer& output, csize_t outputOffset, bool doForward) const {
      if (doForward)
        forwardPlan_->execute (queue, input, inputOffset, output, outputOffset, doForward);
      else
        backwardPlan_->execute (queue, input, inputOffset, output, outputOffset, doForward);
    }
  };

  template <typename F> class GpuFFTPlanFactory {
    bool supportBidirectionalPlan_;
    bool supportNonPOTSizes_;

  protected:
    GpuFFTPlanFactory (bool supportBidirectionalPlan, bool supportNonPOTSizes) : supportBidirectionalPlan_ (supportBidirectionalPlan), supportNonPOTSizes_ (supportNonPOTSizes) {
    }

    virtual boost::shared_ptr<GpuFFTPlan<F> > doCreatePlan (const OpenCL::StubPool& pool, const cl::Device& device, csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward, OpenCL::VectorAccounting& accounting) const = 0;

  public:
    bool supportBidirectionalPlan () const {
      return supportBidirectionalPlan_;
    }

    bool supportNonPOTSizes () const {
      return supportNonPOTSizes_;
    }

    virtual ~GpuFFTPlanFactory ();

    boost::shared_ptr<GpuFFTPlan<F> > createPlan (const OpenCL::StubPool& pool, const cl::Device& device, csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward, OpenCL::VectorAccounting& accounting = OpenCL::VectorAccounting::getNull ()) const {
      ASSERT (inPlace || outOfPlace);
      ASSERT (forward || backward);

      bool isPOT = size > 0 && (size () & (size () - 1)) == 0;
      ASSERT (supportNonPOTSizes () || isPOT || size == 0);

      if (forward && backward && !supportBidirectionalPlan ()) {
        boost::shared_ptr<GpuFFTPlan<F> > forwardPlan = doCreatePlan (pool, device, size, batchCount, inPlace, outOfPlace, true, false, accounting);
        ASSERT (forwardPlan);
        ASSERT (forwardPlan->context () () == pool.context () ());
        ASSERT (forwardPlan->size () == size);
        ASSERT (forwardPlan->batchCount () == batchCount);
        ASSERT (forwardPlan->inPlace () == inPlace);
        ASSERT (forwardPlan->outOfPlace () == outOfPlace);
        ASSERT (forwardPlan->forward ());
        ASSERT (!forwardPlan->backward ());

        boost::shared_ptr<GpuFFTPlan<F> > backwardPlan = doCreatePlan (pool, device, size, batchCount, inPlace, outOfPlace, false, true, accounting);
        ASSERT (backwardPlan);
        ASSERT (backwardPlan->context () () == pool.context () ());
        ASSERT (backwardPlan->size () == size);
        ASSERT (backwardPlan->batchCount () == batchCount);
        ASSERT (backwardPlan->inPlace () == inPlace);
        ASSERT (backwardPlan->outOfPlace () == outOfPlace);
        ASSERT (!backwardPlan->forward ());
        ASSERT (backwardPlan->backward ());

        boost::shared_ptr<GpuFFTPlan<F> > plan (new GpuFFTPlanPair<F> (forwardPlan, backwardPlan, size, batchCount, inPlace, outOfPlace));
        ASSERT (plan->context () () == pool.context () ());
        ASSERT (plan->size () == size);
        ASSERT (plan->batchCount () == batchCount);
        ASSERT (plan->inPlace () == inPlace);
        ASSERT (plan->outOfPlace () == outOfPlace);
        ASSERT (plan->forward () == forward);
        ASSERT (plan->backward () == backward);

        return plan;
      } else {
        boost::shared_ptr<GpuFFTPlan<F> > plan = doCreatePlan (pool, device, size, batchCount, inPlace, outOfPlace, forward, backward, accounting);
        ASSERT (plan);
        ASSERT (plan->context () () == pool.context () ());
        ASSERT (plan->size () == size);
        ASSERT (plan->batchCount () == batchCount);
        ASSERT (plan->inPlace () == inPlace);
        ASSERT (plan->outOfPlace () == outOfPlace);
        ASSERT (plan->forward () == forward);
        ASSERT (plan->backward () == backward);

        return plan;
      }
    }
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, GpuFFTPlan)
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, GpuFFTPlanPair)
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, GpuFFTPlanFactory)
}

#endif // !LINALG_GPUFFTPLAN_HPP_INCLUDED
