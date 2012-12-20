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

#include "Cgnr.hpp"

#include <Core/Time.hpp>

#include <LinAlg/LinComb.hpp>

#include <sstream>
#include <iomanip>

namespace DDA {
  template <typename F> Cgnr<F>::Cgnr (const DDAParams<ftype>& ddaParams, MatVec<ftype>& matVec, csize_t maxIter) : 
    CpuIterativeSolver<F> (ddaParams, matVec, 10, maxIter)
  {
  }
  template <typename F> Cgnr<F>::~Cgnr () {}

  template <typename F> void Cgnr<F>::init (UNUSED std::ostream& log, UNUSED Core::ProfilingDataPtr prof) {
    vars.ro_old = 0;
  }

  template <typename F> F Cgnr<F>::iteration (csize_t nr, UNUSED std::ostream& log, UNUSED bool profilingRun, Core::ProfilingDataPtr prof) {
    std::vector<ctype>& rvec = this->rvec ();
    std::vector<ctype>& Avecbuffer = this->Avecbuffer ();
    std::vector<ctype>& xvec = this->xvec ();
    std::vector<ctype>& pvec = this->tmpVec1 ();

    if (nr == 0) {
      {
        Core::ProfileHandle _p1 (prof, "matvec1");
        this->matVec ().apply (rvec, pvec, true, prof);
      }
      vars.ro_new = LinAlg::norm (pvec);
    } else {
      {
        Core::ProfileHandle _p1 (prof, "matvec1");
        this->matVec ().apply (rvec, Avecbuffer, true, prof);
      }
      vars.ro_new = LinAlg::norm (Avecbuffer);
      vars.beta = vars.ro_new / vars.ro_old;
      LinAlg::linComb (pvec, vars.beta, Avecbuffer, pvec);
    }
    {
      Core::ProfileHandle _p1 (prof, "matvec2");
      this->matVec ().apply (pvec, Avecbuffer, false, prof);
    }
    ctype alpha = vars.ro_new / LinAlg::norm (Avecbuffer);
    LinAlg::linComb (pvec, alpha, xvec, xvec);
    LinAlg::linComb (Avecbuffer, -alpha, rvec, rvec);
    vars.ro_old = vars.ro_new;
    return LinAlg::norm (rvec);
  }


  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, Cgnr)
}
