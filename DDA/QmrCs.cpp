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

#include "QmrCs.hpp"

#include <Core/Time.hpp>

#include <LinAlg/LinComb.hpp>

#include <DDA/Debug.hpp>

#include <sstream>
#include <iomanip>

namespace DDA {
  //#define INFO(x) Debug::info (#x, std::vector<cl::CommandQueue> (), x)
#define INFO(x) do { } while (0)

  template <typename F> QmrCs<F>::QmrCs (const DDAParams<ftype>& ddaParams, MatVec<ftype>& matVec, csize_t maxIter) : 
    CpuIterativeSolver<F> (ddaParams, matVec, 50000, maxIter),
    tmpVec2_ (g ().vecSize ()),
    tmpVec3_ (g ().vecSize ()),
    tmpVec4_ (g ().vecSize ())
  {
  }
  template <typename F> QmrCs<F>::~QmrCs () {}

  namespace {
    template <typename F> std::complex<F> vecProdConj (const std::vector<std::complex<F> >& v1, const std::vector<std::complex<F> >& v2) {
      ASSERT (v1.size () == v2.size ());

      std::complex<F> sum = 0;
      for (size_t i = 0; i < v1.size (); i++)
        sum += v1[i] * v2[i];
      return sum;
    }
  }

  template <typename F> void QmrCs<F>::init (UNUSED std::ostream& log, UNUSED Core::ProfilingDataPtr prof) {
    std::vector<ctype>& v = this->tmpVec1 ();

    ctype rvec2 = vecProdConj (this->rvec (), this->rvec ());
    vars.omega_old = 0;
    vars.beta = std::sqrt (rvec2);
    vars.mBeta = -vars.beta;
    vars.omega_new = std::sqrt (this->inprodR ()) / std::abs (vars.beta);
    vars.tautilda = vars.omega_new * vars.beta;
    vars.c_old = 1;
    vars.c_new = 1;
    vars.s_old = 0;
    vars.s_new = 0;

    LinAlg::linComb (this->rvec (), Const::one / vars.beta, v);
    INFO (this->rvec ());
    INFO (Const::one / vars.beta);
    INFO (v);
  }

  template <typename F> F QmrCs<F>::iteration (csize_t nr, UNUSED std::ostream& log, UNUSED bool profilingRun, Core::ProfilingDataPtr prof) {
    INFO ("");
    std::vector<ctype>& rvec = this->rvec ();
    std::vector<ctype>& Avecbuffer = this->Avecbuffer ();
    std::vector<ctype>& xvec = this->xvec ();

    std::vector<ctype>& v = this->tmpVec1 ();
    std::vector<ctype>& vtilda = this->tmpVec2 ();
    std::vector<ctype>& p_old = this->tmpVec3 ();
    std::vector<ctype>& p_new = this->tmpVec4 ();

    ftype rtmp1 = norm (vars.beta) * this->residScale;
    if (nr == 0)
      ASSERT (!(rtmp1 > 1e+38f) || profilingRun); // Allow very low beta values (seen e.g. when using --load-start-dip-pol)
    else
      ASSERT (!(rtmp1 < 1e-10f || rtmp1 > 1e+38f) || profilingRun);
  
    INFO (rtmp1);

    {
      Core::ProfileHandle _p1 (prof, "matvec");
      this->matVec ().apply (v, Avecbuffer, false, prof);
    }

    ctype alpha = vecProdConj (v, Avecbuffer);

    INFO (vtilda);
    
    if (nr == 0)
      LinAlg::linComb (v, -alpha, Avecbuffer, vtilda);
    else
      LinAlg::linComb (vtilda, vars.mBeta, v, -alpha, Avecbuffer, vtilda);

    INFO (-alpha); INFO (vars.mBeta); INFO (v); INFO (Avecbuffer); INFO (vtilda);

    ctype ctmp3 = vecProdConj (vtilda, vtilda);
    ftype rtmp2 = LinAlg::norm (vtilda);


    ctype ctmp1 = vars.omega_old * vars.beta;
    ctype ctmp2 = vars.omega_new * alpha;

    ctype theta = conj (vars.s_old) * ctmp1;
    ctype eta = vars.c_old * vars.c_new * ctmp1 + conj (vars.s_new) * ctmp2;
    ctype zetatilda = vars.c_new * ctmp2 - vars.c_old * vars.s_new * ctmp1;
    vars.beta = std::sqrt (ctmp3);
    vars.mBeta = -vars.beta;
    vars.omega_old = vars.omega_new;
    vars.omega_new = std::sqrt (rtmp2) / abs (vars.beta);
    ftype zetaabs = std::sqrt (norm (zetatilda) + rtmp2);

    ftype rtmp3 = std::sqrt (norm (zetatilda));
    ctype zeta;
    if (rtmp3 < 1e-40f)
      zeta = zetaabs;
    else
      zeta = zetaabs / rtmp3 * zetatilda;
    vars.c_old = vars.c_new;
    vars.c_new = rtmp3 / zetaabs;
    vars.s_old = vars.s_new;
    vars.s_new = vars.omega_new * (vars.beta / zeta);

    ctype tau = vars.c_new * vars.tautilda;
    vars.tautilda = -vars.s_new * vars.tautilda;

    ctype zetaInv = Const::one / zeta;
    ctype mEtaZeta = -eta / zeta;
    ctype mThetaZeta = -theta / zeta;
    ctype betaInv = Const::one / vars.beta;
    ftype normSNew = norm (vars.s_new);
    ctype cNewOmegaNewTautilda = (vars.c_new / vars.omega_new) * vars.tautilda;

    INFO (zetaInv); INFO (mEtaZeta); INFO (mThetaZeta); INFO (betaInv); INFO (normSNew); INFO (cNewOmegaNewTautilda);

    if (nr == 0) {
      LinAlg::linComb (v, zetaInv, p_new);
    } else {
      if (nr == 1)
        LinAlg::linComb (p_new, mEtaZeta, v, zetaInv, p_old);
      else
        LinAlg::linComb (p_old, mThetaZeta, p_new, mEtaZeta, v, zetaInv, p_old);
      swap (p_old, p_new);
    }

    LinAlg::linComb (p_new, tau, xvec, xvec);
    LinAlg::linComb (vtilda, betaInv, vtilda);
    swap (v, vtilda);
    LinAlg::linComb (rvec, ctype (normSNew), v, cNewOmegaNewTautilda, rvec);
    INFO (p_new); INFO (p_old); INFO (xvec); INFO (vtilda); INFO (v); INFO (rvec);
    return LinAlg::norm (rvec);
  }


  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, QmrCs)
}
