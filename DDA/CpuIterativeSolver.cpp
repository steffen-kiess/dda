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

#include "CpuIterativeSolver.hpp"

#include <LinAlg/LinComb.hpp>

namespace DDA {
  template <class F> CpuIterativeSolver<F>::CpuIterativeSolver (const DDAParams<ftype>& ddaParams, MatVec<ftype>& matVec, csize_t maxResIncrease, csize_t maxIter) :
    IterativeSolverBase<ftype> (ddaParams, maxResIncrease, maxIter),
    matVec_ (matVec),
    Avecbuffer_ (g ().vecSize ()),
    rvec_ (g ().vecSize ()),
    xvec_ (g ().vecSize ()),
    tmpVec1_ (g ().vecSize ())
  {
    ASSERT (g ().procs () == 1);
    ASSERT (&ddaParams == &matVec.ddaParams ());
  }
  template <class F> CpuIterativeSolver<F>::~CpuIterativeSolver () {}

  template <class F> void CpuIterativeSolver<F>::setCoupleConstants (const boost::shared_ptr<const CoupleConstants<ftype> >& cc) {
    matVec ().setCoupleConstants (cc);
  }

  template <class F> F CpuIterativeSolver<F>::initGeneral (const std::vector<ctype>& einc, std::ostream& log, const std::vector<ctype>& start, UNUSED Core::ProfilingDataPtr prof) {
    std::vector<ctype>& pvec = tmpVec1 ();
    for (int j = 0; j < 3; j++)
      for (uint32_t i = g ().nvCount (); i < g ().vecStride (); i++)
        pvec[i + j * g ().vecStride ()] =  0;
    g ().multMat (matVec ().cc ().cc_sqrt (), einc, pvec);

    ftype temp = LinAlg::norm (pvec);
    this->residScale = 1 / temp;

    ftype inprodR = 0.0 / 0.0;

    if (start.size () != 0) {
      std::vector<ctype>& xvec = this->xvec ();
      g ().multMatInv (matVec ().cc ().cc_sqrt (), start, xvec); // xvec = start / cc_sqrt
      std::vector<ctype>& Avecbuffer = this->Avecbuffer ();
      matVec ().apply (xvec, Avecbuffer, false);
      std::vector<ctype>& rvec = this->rvec ();
      LinAlg::linComb (Avecbuffer, ctype (-1), pvec, rvec);
      inprodR = LinAlg::norm (rvec);
      log << "Use loaded start value" << std::endl;
    } else {

      std::vector<ctype>& Avecbuffer = this->Avecbuffer ();
      matVec ().apply (pvec, Avecbuffer, false);

      std::vector<ctype>& rvec = this->rvec ();
      LinAlg::linComb (Avecbuffer, ctype (-1), pvec, rvec);
      inprodR = LinAlg::norm (rvec);

      log << "temp = " << temp << ", inprodR = " << inprodR << std::endl;

      std::vector<ctype>& xvec = this->xvec ();
      if (temp < inprodR) {
        log << "Use 0" << std::endl;
        LinAlg::fill<ctype> (xvec, 0);
        swap (rvec, pvec);
        inprodR = temp;
      } else {
        log << "Use pvec" << std::endl;
        swap (xvec, pvec);
      }
    }

    log << "|r_0|^2: " << temp << std::endl;
    return inprodR;
  }

  template <class F> boost::shared_ptr<std::vector<std::complex<F> > > CpuIterativeSolver<F>::getResult (UNUSED std::ostream& log, UNUSED Core::ProfilingDataPtr prof) {
    std::vector<ctype>& pvec = tmpVec1 ();
    std::vector<ctype>& xvec = this->xvec ();

    g ().multMat (matVec ().cc ().cc_sqrt (), xvec, pvec);
    return boost::make_shared<std::vector<std::complex<F> > > (pvec);
  }


  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, CpuIterativeSolver)
}
