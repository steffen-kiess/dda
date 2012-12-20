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

#include "IterativeSolverBase.hpp"

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

namespace DDA {
  template <class F> IterativeSolverBase<F>::IterativeSolverBase (const DDAParams<ftype>& ddaParams, csize_t maxResIncrease, csize_t maxIter)
    : ddaParams_ (ddaParams),
      initTime (0),
      count (0),
      counter (0),
      maxResIncrease (maxResIncrease),
      maxIter (maxIter)
  {
  }
  template <class F> IterativeSolverBase<F>::~IterativeSolverBase () {}

  template <class F> boost::shared_ptr<std::vector<std::complex<F> > > IterativeSolverBase<F>::getPolVec (const std::vector<ctype>& einc, ftype eps, std::ostream& log, const std::vector<ctype>& start, Core::ProfilingDataPtr prof) {
    this->inprodR_ = initGeneral (einc, log, start, prof);
    log << "inprodR = " << this->inprodR_ << std::endl;

    this->epsB = eps * eps / this->residScale;
    this->prev_err = std::sqrt (this->residScale * this->inprodR_);
    this->count = 0;
    this->counter = 0;

    this->inprodRInit = this->inprodR_;
    this->needNewline = false;
    this->initTime = Core::getCurrentTime ();

    {
      Core::ProfileHandle _p1 (prof, "itsolv");
      init (log, prof);
      while (inprodR_ >= epsB /* && count + 1 <= maxIter && counter <= maxResIncrease */) {
        if (this->count >= maxIter) {
          ABORT_MSG ("Too many iterations");
        } else if (this->counter > this->maxResIncrease) {
          ABORT_MSG ("Too many iterations w/o increase");
        }
        ftype inprodRplus1 = iteration (count, log, false, prof);
        count++;
        { // LoopUpdate
          if (inprodRplus1 <= inprodR_) {
            inprodR_ = inprodRplus1;
            counter = 0;
          } else {
            counter++;
          }
          ftype err = std::sqrt (residScale * inprodRplus1);
          ftype progr = 1 - err / prev_err;
          std::stringstream progStr;
          ftype prog = (std::log (this->inprodR_) - std::log (this->inprodRInit)) / (std::log (epsB) - std::log (this->inprodRInit));
          Core::TimeSpan now = Core::getCurrentTime ();
          Core::TimeSpan remain = (now - this->initTime) * static_cast<double> ((1 - prog) / prog);
          progStr << "RE_" << std::setfill ('0') << std::setw (5) << count << " = " << std::scientific << std::setfill (' ') << std::setw (12) << err << "  ";
          if (counter == 0)
            progStr << "+ ";
          else if (progr > 0)
            progStr << "-+";
          else
            progStr << "- ";
          progStr << "  [" << std::fixed << std::setprecision (2) << std::setw (6) << prog * 100 << "%]  [" << std::setfill (' ') << std::setw (12) << remain.toString () << "]";
          if (this->needNewline)
            Core::OStream::getStderr () << "\n";
          Core::OStream::getStderr () << progStr.str ();
          /*
            this->needNewline = count <= 1
            || (count < 100 && count % 10 == 0)
            || (count <= 1000 && count % 100 == 0);
          */
          this->needNewline = false;
          Core::OStream::getStderr () << "\r" << std::flush;
          log << progStr.str () << std::endl;
          prev_err = err;
        }
      }
      Core::OStream::getStderr () << std::endl;
    }

    return getResult (log, prof);
  }

  template <class F> void IterativeSolverBase<F>::profilingRun (std::ostream& out, std::ostream& log, Core::ProfilingDataPtr prof) {
    this->epsB = 1;
    this->prev_err = 1;
    this->count = 10;
    this->counter = 0;

    {
      Core::ProfileHandle _p1 (prof, "itsolv1");
      iteration (count - 1, log, true, prof);
    }

    const int nr = 5;
    const int mid = nr / 2;
    boost::tuple<uint64_t, int> res[nr];
    {
      Core::ProfileHandle _p1 (prof, "itsolv2");
      Core::TimeSpan now = Core::getCurrentTime ();
      for (int i = 0; i < nr; i++) {
        iteration (count + i, log, true, prof);
        res[i] = boost::make_tuple ((Core::getCurrentTime () - now).getMicroseconds (), i);
        now = Core::getCurrentTime ();
      }
    }

    std::sort (res, res + nr);
    for (int i = 0; i < nr; i++)
      out << "[" << res[i].get<1> () << "]" << Core::TimeSpan (res[i].get<0> ()).toString () << " ";
    uint64_t diff = std::max (res[mid].get<0> () - res[0].get<0> (), res[nr - 1].get<0> () - res[mid].get<0> ());
    out << std::endl;
    out << std::endl;
    out << Core::TimeSpan (res[mid].get<0> ()).toString () << " +- " << Core::TimeSpan (diff).toString () << std::endl;
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, IterativeSolverBase)
}
