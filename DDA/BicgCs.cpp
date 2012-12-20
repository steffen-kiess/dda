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

#include "BicgCs.hpp"

#include <Core/Time.hpp>

#include <LinAlg/LinComb.hpp>

#include <sstream>
#include <iomanip>

namespace DDA {
  namespace {
    template <typename F> std::complex<F> vecProdConj (const std::vector<std::complex<F> >& v1, const std::vector<std::complex<F> >& v2) {
      ASSERT (v1.size () == v2.size ());

      std::complex<F> sum = 0;
      for (size_t i = 0; i < v1.size (); i++)
        sum += v1[i] * v2[i];
      return sum;
    }

    template <typename F> void copy (const std::vector<std::complex<F> >& from, std::vector<std::complex<F> >& to) {
      ASSERT (from.size () == to.size ());
      for (size_t i = 0; i < from.size (); i++)
        to[i] = from[i];
    }
  }

  template <typename F> BicgCs<F>::BicgCs (const DDAParams<ftype>& ddaParams, MatVec<ftype>& matVec, csize_t maxIter) : 
    CpuIterativeSolver<F> (ddaParams, matVec, 50000, maxIter)
  {
  }
  template <typename F> BicgCs<F>::~BicgCs () {}

  template <typename F> void BicgCs<F>::init (UNUSED std::ostream& log, UNUSED Core::ProfilingDataPtr prof) {
  }

  template <typename F> F BicgCs<F>::iteration (csize_t nr, UNUSED std::ostream& log, UNUSED bool profilingRun, Core::ProfilingDataPtr prof) {
    std::vector<ctype>& rvec = this->rvec ();
    std::vector<ctype>& Avecbuffer = this->Avecbuffer ();
    std::vector<ctype>& xvec = this->xvec ();
    std::vector<ctype>& pvec = this->tmpVec1 ();

    vars.ro_new = vecProdConj (rvec, rvec);
    vars.abs_ro_new = std::abs (vars.ro_new);
    ftype dtmp = vars.abs_ro_new / this->inprodR ();
    ASSERT (!(dtmp < 1e-10 || dtmp > 1e+10) || profilingRun);
    if (nr == 0) {
      copy (rvec, pvec);
    } else {
      vars.beta = vars.ro_new / vars.ro_old;
      LinAlg::linComb (pvec, vars.beta, rvec, pvec);
    }
    this->matVec ().apply (pvec, Avecbuffer, false, prof);
    ctype mu_k = vecProdConj (pvec, Avecbuffer);
    ftype dtmp2 = std::abs (mu_k) / vars.abs_ro_new;
    ASSERT (!(dtmp2 < 10e-10) || profilingRun);
    vars.alpha = vars.ro_new / mu_k;
    LinAlg::linComb (pvec, vars.alpha, xvec, xvec);
    LinAlg::linComb (Avecbuffer, -vars.alpha, rvec, rvec);
    vars.ro_old = vars.ro_new;
    return LinAlg::norm (rvec);
  }


  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, BicgCs)
}
