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

#include "FFTWPlan.hpp"

#include <fftw3.h>

#include <boost/type_traits/alignment_of.hpp>
#include <boost/type_traits/aligned_storage.hpp>

namespace LinAlg {
  namespace {
    template <typename F> struct FFTWOperations;

    template <typename F> class FFTWPlan : public FFTPlan<F> {
      void* p;

    public:  
      FFTWPlan (csize_t size, csize_t batchCount, bool inPlace, bool forward, bool has128BitAlignment);

      virtual ~FFTWPlan ();

    protected:
      virtual void doExecute (const std::complex<F>* input, std::complex<F>* output, bool doForward, Core::ProfilingDataPtr prof) const;
    };

    template <typename F> class FFTWPlanFactory : public FFTPlanFactory<F> {
    public:
      FFTWPlanFactory () : FFTPlanFactory<F> (false, true) {
      }

    protected:
      virtual boost::shared_ptr<FFTPlan<F> > doCreatePlan (csize_t size, csize_t batchCount, bool inPlace, bool outOfPlace, bool forward, bool backward, bool has128BitAlignment) const {
        ASSERT (!inPlace || !outOfPlace);
        ASSERT (!forward || !backward);

        return boost::shared_ptr<FFTPlan<F> > (new FFTWPlan<F> (size, batchCount, inPlace, forward, has128BitAlignment));
      }
    };
  }

  template <typename F> const FFTPlanFactory<F>& getFFTWPlanFactory () {
    static FFTWPlanFactory<F> factory;
    return factory;
  }

#define TY(x) typename FFTWOperations<T>::x
#define FUN(x) FFTWOperations<T>::x ()

  template <class T> FFTWPlan<T>::FFTWPlan (csize_t size, csize_t batchCount, bool inPlace, bool forward, bool has128BitAlignment) : FFTPlan<T> (size, batchCount, inPlace, !inPlace, forward, !forward, has128BitAlignment) {
#define MAX(x, y) ((x) > (y) ? (x) : (y))
    typedef boost::aligned_storage<sizeof (TY(complex)), MAX(16, boost::alignment_of<TY(complex)>::value)> AlignedType;
#undef MAX
    AlignedType in;
    AlignedType out;

    ASSERT (((uintptr_t) &in) % 16 == 0);
    ASSERT (((uintptr_t) &out) % 16 == 0);

    if (size >= 1) {
      int sizeInt = Core::checked_cast<int> (size);
      int batchCountInt = Core::checked_cast<int> (batchCount);
      TY(plan) plan = FUN(plan_many_dft) (1, &sizeInt, batchCountInt, (TY(complex)*) &in, NULL, 1, sizeInt, inPlace ? (TY(complex)*) &in : (TY(complex)*) &out, NULL, 1, sizeInt, forward ? FFTW_FORWARD : FFTW_BACKWARD, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT | (has128BitAlignment ? 0 : FFTW_UNALIGNED));
      ASSERT (plan != NULL);

      p = (void*) plan;
    } else {
      p = NULL;
    }
  }

  template <class T> FFTWPlan<T>::~FFTWPlan () {
    if (p)
      FUN(destroy_plan) ((TY(plan)) p);
  }

  template <class T> void FFTWPlan<T>::doExecute (const std::complex<T>* input,
                                                  std::complex<T>* output,
                                                  bool doForward, UNUSED Core::ProfilingDataPtr prof) const {
    ASSERT (doForward == this->forward ());

    if (this->size () == 0)
      return;
    if (this->size () == 1) {
      if (input != output)
        for (size_t i = 0; i < this->batchCount (); i++)
          output[i] = input[i];
      return;
    }

    TY(complex)* out = (TY(complex)*) output;
    TY(complex)* in = (TY(complex)*) input;

    FUN(execute_dft) ((TY(plan)) p, in, out);
  }


#define DT(P, name) typedef P##name name;
#define D(P, name) static inline __typeof__ (&P##name) name () { return P##name; }

#define I(F, P)                                                 \
  namespace {                                                   \
    template <> struct FFTWOperations<F> {                      \
      DT (P, complex)                                           \
      DT (P, plan)                                              \
      D (P, execute_dft)                                        \
      D (P, plan_many_dft)                                      \
      D (P, destroy_plan)                                       \
    };                                                          \
  }                                                             \
  template const FFTPlanFactory<F>& getFFTWPlanFactory ();

  I (float, fftwf_)
  I (double, fftw_)
  I (long double, fftwl_)

#undef I
#undef D
#undef DT
}
