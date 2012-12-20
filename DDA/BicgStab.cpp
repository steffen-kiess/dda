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

#include "BicgStab.hpp"

#include <Core/Time.hpp>

#include <LinAlg/LinComb.hpp>

#include <sstream>
#include <iomanip>

namespace DDA {
  namespace {
    template <typename F> std::complex<F> vecProd (const std::vector<std::complex<F> >& v1, const std::vector<std::complex<F> >& v2) {
      ASSERT (v1.size () == v2.size ());

      std::complex<F> sum = 0;
      for (size_t i = 0; i < v1.size (); i++)
        sum += v1[i] * std::conj (v2[i]);
      return sum;
    }

    template <typename F> void copy (const std::vector<std::complex<F> >& from, std::vector<std::complex<F> >& to) {
      ASSERT (from.size () == to.size ());
      for (size_t i = 0; i < from.size (); i++)
        to[i] = from[i];
    }
  }

  template <typename F> BicgStab<F>::BicgStab (const DDAParams<ftype>& ddaParams, MatVec<ftype>& matVec, csize_t maxIter) : 
    CpuIterativeSolver<F> (ddaParams, matVec, 30000, maxIter),
    tmpVec2_ (g ().vecSize ()),
    tmpVec3_ (g ().vecSize ()),
    tmpVec4_ (g ().vecSize ())
  {
  }
  template <typename F> BicgStab<F>::~BicgStab () {}

  template <typename F> void BicgStab<F>::init (UNUSED std::ostream& log, UNUSED Core::ProfilingDataPtr prof) {
    std::vector<ctype>& rvec = this->rvec ();
    std::vector<ctype>& rtilda = this->tmpVec4 ();

    copy (rvec, rtilda);
  }

  template <typename F> F BicgStab<F>::iteration (csize_t nr, UNUSED std::ostream& log, bool profilingRun, Core::ProfilingDataPtr prof) {
    std::vector<ctype>& rvec = this->rvec ();
    std::vector<ctype>& Avecbuffer = this->Avecbuffer ();
    std::vector<ctype>& xvec = this->xvec ();

    std::vector<ctype>& pvec = this->tmpVec1 ();
    std::vector<ctype>& v = this->tmpVec2 ();
    std::vector<ctype>& s = this->tmpVec3 ();
    std::vector<ctype>& rtilda = this->tmpVec4 ();

    vars.ro_new = vecProd (rvec, rtilda);
    // Use higher precision to avoid underflow / overflow
    ftype dtmp = static_cast<ftype> (std::abs<ldouble> (vars.ro_new) / this->inprodR ());
    ASSERT (dtmp >= 1e-16 || profilingRun);
    if (nr == 0) {
      copy (rvec, pvec);
    } else {
      // Use higher precision to avoid underflow / overflow
      cldouble ro_new_alpha = vars.ro_new * vars.alpha;
      cldouble ro_old_omega = vars.ro_old * vars.omega;
      ASSERT (std::abs (ro_old_omega) / std::abs (ro_new_alpha) >= 10e-10 || profilingRun);
      vars.beta = static_cast<ctype> (ro_new_alpha / ro_old_omega);
      LinAlg::linComb (pvec, vars.beta, v, -vars.beta * vars.omega, rvec, pvec);
    }
    this->matVec ().apply (pvec, v, false, prof);
    // Use higher precision to avoid underflow / overflow
    //vars.alpha = vars.ro_new / vecProd (v, rtilda);
    cldouble ro_new = vars.ro_new;
    cldouble vRtilda = vecProd (v, rtilda);
    vars.alpha = static_cast<ctype> (ro_new / vRtilda);
    LinAlg::linComb (v, -vars.alpha, rvec, s);
    ftype inprodRplus1 = LinAlg::norm (s);
    if (inprodRplus1 < this->epsB && !profilingRun) {
      LinAlg::linComb (pvec, vars.alpha, xvec, xvec);
    } else {
      this->matVec ().apply (s, Avecbuffer, false, prof);
      vars.denumOmega = LinAlg::norm (Avecbuffer);
      // Use higher precision to avoid underflow / overflow
      //vars.omega = vecProd (s, Avecbuffer) / vars.denumOmega;
      vars.omega = static_cast<ctype> (static_cast<cldouble> (vecProd (s, Avecbuffer)) / static_cast<ldouble> (vars.denumOmega));
      LinAlg::linComb (pvec, vars.alpha, s, vars.omega, xvec, xvec);
      LinAlg::linComb (Avecbuffer, -vars.omega, s, rvec);
      inprodRplus1 = LinAlg::norm (rvec);
      vars.ro_old = vars.ro_new;
    }
    return inprodRplus1;
  }


  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, BicgStab)
}
