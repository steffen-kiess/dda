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

#ifndef LINALG_FFTPLAN_HPP_INCLUDED
#define LINALG_FFTPLAN_HPP_INCLUDED

// LinAlg::FFTPlan is an abstract class for a 1d-fft plan running on the CPU
//
// LinAlg::FFTPlanFactory is an abstract factory of FFTPlans

#include <Core/Assert.hpp>
#include <Core/Profiling.hpp>
#include <Core/CheckedIntegerAlias.hpp>

#include <Math/FPTemplateInstances.hpp>

#include <complex>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/multi_array.hpp>

namespace LinAlg {
  template <typename F> class FFTPlan {
    csize_t _size;
    csize_t _batchCount;
    csize_t _batchSize;
    bool _inPlace;
    bool _outOfPlace;
    bool _forward;
    bool _backward;
    bool _need128BitAlignment;

  public:  
    virtual ~FFTPlan ();

  protected:
    FFTPlan (csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward, bool need128BitAlignment) : _size (size), _batchCount (batchCount), _batchSize (size * batchCount), _inPlace (inPlace), _outOfPlace (outOfPlace), _forward (forward), _backward (backward), _need128BitAlignment (need128BitAlignment) {
      ASSERT (inPlace || outOfPlace);
      ASSERT (forward || backward);
    }

    virtual void doExecute (const std::complex<F>* input, std::complex<F>* output, bool doForward, Core::ProfilingDataPtr prof) const = 0;

  public:
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

    bool need128BitAlignment () const {
      return _need128BitAlignment;
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

    void execute (const std::complex<F>* input, std::complex<F>* output, bool doForward, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      if (doForward)
        ASSERT (forward ());
      else
        ASSERT (backward ());

      if (need128BitAlignment ()) {
        ASSERT (((uintptr_t) output) % 16 == 0);
        ASSERT (((uintptr_t) input) % 16 == 0);
      }

      if (input == output) {
        if (batchSize () != 0)
          ASSERT (inPlace ());
      } else {
        ASSERT (outOfPlace ());
        if (input > output) {
          ASSERT (input >= output + batchSize () ());
        } else {
          ASSERT (output >= input + batchSize () ());
        }
      }
    
      doExecute (input, output, doForward, prof);
    }

    void fft (const std::complex<F>* input, std::complex<F>* output, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (input, output, true, prof);
    }

    void ifft (const std::complex<F>* input, std::complex<F>* output, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (input, output, false, prof);
    }

    void executeOutOfPlace (const std::complex<F>* input, std::complex<F>* output, bool doForward, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      ASSERT (input != output);
      execute (input, output, doForward, prof);
    }

    void fftOutOfPlace (const std::complex<F>* input, std::complex<F>* output, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeOutOfPlace (input, output, true, prof);
    }

    void ifftOutOfPlace (const std::complex<F>* input, std::complex<F>* output, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeOutOfPlace (input, output, false, prof);
    }

    void executeInPlace (std::complex<F>* inout, bool doForward, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (inout, inout, doForward, prof);
    }

    void fftInPlace (std::complex<F>* inout, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeInPlace (inout, true, prof);
    }

    void ifftInPlace (std::complex<F>* inout, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeInPlace (inout, false, prof);
    }


    // FFT methods for 1D-Vector
    template <typename AIn, typename AOut>
    void execute (const std::vector<std::complex<F>, AIn>& input, std::vector<std::complex<F>, AOut>& output, bool doForward, csize_t inputOffset, csize_t outputOffset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      ASSERT (input.size () >= inputOffset + batchSize ());
      ASSERT (output.size () >= outputOffset + batchSize ());

      if (input.data () == output.data () && inputOffset == outputOffset) {
        ASSERT (inPlace ());
      } else {
        ASSERT (outOfPlace ());
      }

      execute (input.data (), output.data (), doForward, prof);
    }
    template <typename AIn, typename AOut>
    void execute (const std::vector<std::complex<F>, AIn>& input, std::vector<std::complex<F>, AOut>& output, bool doForward, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      ASSERT (input.size () == batchSize ());
      ASSERT (output.size () == batchSize ());
      execute (input, output, doForward, 0, 0, prof);
    }

    template <typename AIn, typename AOut>
    void fft (const std::vector<std::complex<F>, AIn>& input, std::vector<std::complex<F>, AOut>& output, csize_t inputOffset, csize_t outputOffset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (input, output, true, inputOffset, outputOffset, prof);
    }
    template <typename AIn, typename AOut>
    void fft (const std::vector<std::complex<F>, AIn>& input, std::vector<std::complex<F>, AOut>& output, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (input, output, true, prof);
    }

    template <typename AIn, typename AOut>
    void ifft (const std::vector<std::complex<F>, AIn>& input, std::vector<std::complex<F>, AOut>& output, csize_t inputOffset, csize_t outputOffset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (input, output, false, inputOffset, outputOffset, prof);
    }
    template <typename AIn, typename AOut>
    void ifft (const std::vector<std::complex<F>, AIn>& input, std::vector<std::complex<F>, AOut>& output, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (input, output, false, prof);
    }

    template <typename AIn, typename AOut>
    void executeOutOfPlace (const std::vector<std::complex<F>, AIn>& input, std::vector<std::complex<F>, AOut>& output, bool doForward, csize_t inputOffset, csize_t outputOffset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      ASSERT (input.data () != output.data () || inputOffset != outputOffset);
      execute (input, output, doForward, inputOffset, outputOffset, prof);
    }
    template <typename AIn, typename AOut>
    void executeOutOfPlace (const std::vector<std::complex<F>, AIn>& input, std::vector<std::complex<F>, AOut>& output, bool doForward, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      ASSERT (input.data () != output.data ());
      execute (input, output, doForward, prof);
    }

    template <typename AIn, typename AOut>
    void fftOutOfPlace (const std::vector<std::complex<F>, AIn>& input, std::vector<std::complex<F>, AOut>& output, csize_t inputOffset, csize_t outputOffset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeOutOfPlace (input, output, true, inputOffset, outputOffset, prof);
    }
    template <typename AIn, typename AOut>
    void fftOutOfPlace (const std::vector<std::complex<F>, AIn>& input, std::vector<std::complex<F>, AOut>& output, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeOutOfPlace (input, output, true, prof);
    }

    template <typename AIn, typename AOut>
    void ifftOutOfPlace (const std::vector<std::complex<F>, AIn>& input, std::vector<std::complex<F>, AOut>& output, csize_t inputOffset, csize_t outputOffset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeOutOfPlace (input, output, false, inputOffset, outputOffset, prof);
    }
    template <typename AIn, typename AOut>
    void ifftOutOfPlace (const std::vector<std::complex<F>, AIn>& input, std::vector<std::complex<F>, AOut>& output, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeOutOfPlace (input, output, false, prof);
    }

    template <typename Alloc>
    void executeInPlace (std::vector<std::complex<F>, Alloc>& inout, bool doForward, csize_t offset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (inout, inout, doForward, offset, offset, prof);
    }
    template <typename Alloc>
    void executeInPlace (std::vector<std::complex<F>, Alloc>& inout, bool doForward, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (inout, inout, doForward, prof);
    }

    template <typename Alloc>
    void fftInPlace (std::vector<std::complex<F>, Alloc>& inout, csize_t offset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeInPlace (inout, true, offset, prof);
    }
    template <typename Alloc>
    void fftInPlace (std::vector<std::complex<F>, Alloc>& inout, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeInPlace (inout, true, prof);
    }

    template <typename Alloc>
    void ifftInPlace (std::vector<std::complex<F>, Alloc>& inout, csize_t offset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeInPlace (inout, false, offset, prof);
    }
    template <typename Alloc>
    void ifftInPlace (std::vector<std::complex<F>, Alloc>& inout, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeInPlace (inout, false, prof);
    }

    template <typename AIn>
    boost::shared_ptr<std::vector<std::complex<F> > > execute (const std::vector<std::complex<F>, AIn>& input, bool doForward, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      ASSERT (input.size () == batchSize ());

      if (outOfPlace ()) {
        boost::shared_ptr<std::vector<std::complex<F> > > output = boost::make_shared<std::vector<std::complex<F> > > (batchSize () ());
        executeOutOfPlace (input, *output, doForward, prof);
        return output;
      } else {
        boost::shared_ptr<std::vector<std::complex<F> > > output = boost::make_shared<std::vector<std::complex<F> > > (input);
        executeInPlace (*output, doForward, prof);
        return output;
      }
    }

    template <typename AIn>
    boost::shared_ptr<std::vector<std::complex<F> > > fft (const std::vector<std::complex<F>, AIn>& input, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      return execute (input, true, prof);
    }

    template <typename AIn>
    boost::shared_ptr<std::vector<std::complex<F> > > ifft (const std::vector<std::complex<F>, AIn>& input, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      return execute (input, true, prof);
    }


    // FFT methods for 2D-Vector, does FFT on first dimension with batch on second
    void execute (const boost::const_multi_array_ref<std::complex<F>, 2>& input, boost::multi_array_ref<std::complex<F>, 2>& output, bool doForward, csize_t inputOffset, csize_t outputOffset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      ASSERT (input.shape ()[0] == size ());
      ASSERT (output.shape ()[0] == size ());

      ASSERT (input.shape ()[1] >= inputOffset + batchCount ());
      ASSERT (output.shape ()[1] >= outputOffset + batchCount ());

      ASSERT (input.strides ()[0] == 1);
      ASSERT (output.strides ()[0] == 1);
      ASSERT (input.strides ()[1] == size ());
      ASSERT (output.strides ()[1] == size ());

      if (input.data () == output.data () && inputOffset == outputOffset) {
        ASSERT (inPlace ());
      } else {
        ASSERT (outOfPlace ());
      }

      execute (input.data () + (inputOffset * size ()) (), output.data () + (outputOffset * size ()) (), doForward, prof);
    }
    void execute (const boost::const_multi_array_ref<std::complex<F>, 2>& input, boost::multi_array_ref<std::complex<F>, 2>& output, bool doForward, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      ASSERT (input.shape ()[1] == batchCount ());
      ASSERT (output.shape ()[1] == batchCount ());
      execute (input, output, doForward, 0, 0, prof);
    }

    void fft (const boost::const_multi_array_ref<std::complex<F>, 2>& input, boost::multi_array_ref<std::complex<F>, 2>& output, csize_t inputOffset, csize_t outputOffset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (input, output, true, inputOffset, outputOffset, prof);
    }
    void fft (const boost::const_multi_array_ref<std::complex<F>, 2>& input, boost::multi_array_ref<std::complex<F>, 2>& output, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (input, output, true, prof);
    }

    void ifft (const boost::const_multi_array_ref<std::complex<F>, 2>& input, boost::multi_array_ref<std::complex<F>, 2>& output, csize_t inputOffset, csize_t outputOffset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (input, output, false, inputOffset, outputOffset, prof);
    }
    void ifft (const boost::const_multi_array_ref<std::complex<F>, 2>& input, boost::multi_array_ref<std::complex<F>, 2>& output, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (input, output, false, prof);
    }

    void executeOutOfPlace (const boost::const_multi_array_ref<std::complex<F>, 2>& input, boost::multi_array_ref<std::complex<F>, 2>& output, bool doForward, csize_t inputOffset, csize_t outputOffset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      ASSERT (input.data () != output.data () || inputOffset != outputOffset);
      execute (input, output, doForward, inputOffset, outputOffset, prof);
    }
    void executeOutOfPlace (const boost::const_multi_array_ref<std::complex<F>, 2>& input, boost::multi_array_ref<std::complex<F>, 2>& output, bool doForward, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      ASSERT (input.data () != output.data ());
      execute (input, output, doForward, prof);
    }

    void fftOutOfPlace (const boost::const_multi_array_ref<std::complex<F>, 2>& input, boost::multi_array_ref<std::complex<F>, 2>& output, csize_t inputOffset, csize_t outputOffset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeOutOfPlace (input, output, true, inputOffset, outputOffset, prof);
    }
    void fftOutOfPlace (const boost::const_multi_array_ref<std::complex<F>, 2>& input, boost::multi_array_ref<std::complex<F>, 2>& output, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeOutOfPlace (input, output, true, prof);
    }

    void ifftOutOfPlace (const boost::const_multi_array_ref<std::complex<F>, 2>& input, boost::multi_array_ref<std::complex<F>, 2>& output, csize_t inputOffset, csize_t outputOffset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeOutOfPlace (input, output, false, inputOffset, outputOffset, prof);
    }
    void ifftOutOfPlace (const boost::const_multi_array_ref<std::complex<F>, 2>& input, boost::multi_array_ref<std::complex<F>, 2>& output, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeOutOfPlace (input, output, false, prof);
    }

    void executeInPlace (boost::multi_array_ref<std::complex<F>, 2>& inout, bool doForward, csize_t offset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (inout, inout, doForward, offset, offset, prof);
    }
    void executeInPlace (boost::multi_array_ref<std::complex<F>, 2>& inout, bool doForward, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      execute (inout, inout, doForward, prof);
    }

    void fftInPlace (boost::multi_array_ref<std::complex<F>, 2>& inout, csize_t offset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeInPlace (inout, true, offset, prof);
    }
    void fftInPlace (boost::multi_array_ref<std::complex<F>, 2>& inout, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeInPlace (inout, true, prof);
    }

    void ifftInPlace (boost::multi_array_ref<std::complex<F>, 2>& inout, csize_t offset, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeInPlace (inout, false, offset, prof);
    }
    void ifftInPlace (boost::multi_array_ref<std::complex<F>, 2>& inout, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      executeInPlace (inout, false, prof);
    }

    boost::shared_ptr<boost::multi_array<std::complex<F>, 2> > execute (const boost::const_multi_array_ref<std::complex<F>, 2>& input, bool doForward, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      ASSERT (input.shape ()[0] == size ());
      ASSERT (input.shape ()[1] == batchCount ());

      if (outOfPlace ()) {
        boost::shared_ptr<boost::multi_array<std::complex<F>, 2> > output = boost::make_shared<boost::multi_array<std::complex<F>, 2> > (boost::extents[size () ()][batchSize () ()], boost::fortran_storage_order ());
        executeOutOfPlace (input, *output, doForward, prof);
        return output;
      } else {
        boost::shared_ptr<boost::multi_array<std::complex<F>, 2> > output = boost::make_shared<boost::multi_array<std::complex<F>, 2> > (input);
        executeInPlace (*output, doForward, prof);
        return output;
      }
    }

    boost::shared_ptr<boost::multi_array<std::complex<F>, 2> > fft (const boost::const_multi_array_ref<std::complex<F>, 2>& input, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      return execute (input, true, prof);
    }

    boost::shared_ptr<boost::multi_array<std::complex<F>, 2> > ifft (const boost::const_multi_array_ref<std::complex<F>, 2>& input, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      return execute (input, true, prof);
    }
  };

  template <typename F> class FFTPlanFactory;
  template <typename F> class FFTPlanPair : public FFTPlan<F> {
    friend class FFTPlanFactory<F>;

    boost::shared_ptr<FFTPlan<F> > forwardPlan_;
    boost::shared_ptr<FFTPlan<F> > backwardPlan_;

    FFTPlanPair (const boost::shared_ptr<FFTPlan<F> >& forwardPlan, const boost::shared_ptr<FFTPlan<F> >& backwardPlan, csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool has128BitAlignment) : FFTPlan<F> (size, batchCount, inPlace, outOfPlace, true, true, has128BitAlignment), forwardPlan_ (forwardPlan), backwardPlan_ (backwardPlan) {
      ASSERT (forwardPlan);
      ASSERT (backwardPlan);
    }

  public:
    virtual ~FFTPlanPair ();

  protected:
    virtual void doExecute (const std::complex<F>* input, std::complex<F>* output, bool doForward, Core::ProfilingDataPtr prof) const {
      if (doForward)
        forwardPlan_->execute (input, output, doForward, prof);
      else
        backwardPlan_->execute (input, output, doForward, prof);
    }
  };

  template <typename F> class FFTPlanFactory {
    bool supportBidirectionalPlan_;
    bool supportNonPOTSizes_;

  protected:
    FFTPlanFactory (bool supportBidirectionalPlan, bool supportNonPOTSizes) : supportBidirectionalPlan_ (supportBidirectionalPlan), supportNonPOTSizes_ (supportNonPOTSizes) {
    }

    virtual boost::shared_ptr<FFTPlan<F> > doCreatePlan (csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward, bool has128BitAlignment) const = 0;

  public:
    bool supportBidirectionalPlan () const {
      return supportBidirectionalPlan_;
    }

    bool supportNonPOTSizes () const {
      return supportNonPOTSizes_;
    }

    virtual ~FFTPlanFactory ();

    boost::shared_ptr<FFTPlan<F> > createPlan (csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward, bool has128BitAlignment = false) const {
      ASSERT (inPlace || outOfPlace);
      ASSERT (forward || backward);

      bool isPOT = size > 0 && (size () & (size () - 1)) == 0;
      ASSERT (supportNonPOTSizes () || isPOT || size == 0);

      if (forward && backward && !supportBidirectionalPlan ()) {
        boost::shared_ptr<FFTPlan<F> > forwardPlan = doCreatePlan (size, batchCount, inPlace, outOfPlace, true, false, has128BitAlignment);
        ASSERT (forwardPlan);
        ASSERT (forwardPlan->size () == size);
        ASSERT (forwardPlan->batchCount () == batchCount);
        ASSERT (forwardPlan->inPlace () == inPlace);
        ASSERT (forwardPlan->outOfPlace () == outOfPlace);
        ASSERT (forwardPlan->forward ());
        ASSERT (!forwardPlan->backward ());
        ASSERT (forwardPlan->need128BitAlignment () == has128BitAlignment);

        boost::shared_ptr<FFTPlan<F> > backwardPlan = doCreatePlan (size, batchCount, inPlace, outOfPlace, false, true, has128BitAlignment);
        ASSERT (backwardPlan);
        ASSERT (backwardPlan->size () == size);
        ASSERT (backwardPlan->batchCount () == batchCount);
        ASSERT (backwardPlan->inPlace () == inPlace);
        ASSERT (backwardPlan->outOfPlace () == outOfPlace);
        ASSERT (!backwardPlan->forward ());
        ASSERT (backwardPlan->backward ());
        ASSERT (backwardPlan->need128BitAlignment () == has128BitAlignment);

        boost::shared_ptr<FFTPlan<F> > plan (new FFTPlanPair<F> (forwardPlan, backwardPlan, size, batchCount, inPlace, outOfPlace, has128BitAlignment));
        ASSERT (plan->size () == size);
        ASSERT (plan->batchCount () == batchCount);
        ASSERT (plan->inPlace () == inPlace);
        ASSERT (plan->outOfPlace () == outOfPlace);
        ASSERT (plan->forward () == forward);
        ASSERT (plan->backward () == backward);
        ASSERT (plan->need128BitAlignment () == has128BitAlignment);

        return plan;
      } else {
        boost::shared_ptr<FFTPlan<F> > plan = doCreatePlan (size, batchCount, inPlace, outOfPlace, forward, backward, has128BitAlignment);
        ASSERT (plan);
        ASSERT (plan->size () == size);
        ASSERT (plan->batchCount () == batchCount);
        ASSERT (plan->inPlace () == inPlace);
        ASSERT (plan->outOfPlace () == outOfPlace);
        ASSERT (plan->forward () == forward);
        ASSERT (plan->backward () == backward);
        ASSERT (plan->need128BitAlignment () == has128BitAlignment);

        return plan;
      }
    }
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, FFTPlan)
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, FFTPlanPair)
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, FFTPlanFactory)
}

#endif // !LINALG_FFTPLAN_HPP_INCLUDED
