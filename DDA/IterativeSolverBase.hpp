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

#ifndef DDA_ITERATIVESOLVERBASE_HPP_INCLUDED
#define DDA_ITERATIVESOLVERBASE_HPP_INCLUDED

// Base class for iterative solvers on the CPU or on the GPU

#include <DDA/DDAParams.hpp>

namespace DDA {
  template <typename F>
  class IterativeSolverBase {
    typedef F ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

    const DDAParams<ftype>& ddaParams_;

    ftype inprodRInit;
    bool needNewline;
    Core::TimeSpan initTime;

    ftype inprodR_;
    csize_t count;
    csize_t counter;
    ftype prev_err;

    csize_t maxResIncrease;
    csize_t maxIter;

  public:
    IterativeSolverBase (const DDAParams<ftype>& ddaParams, csize_t maxResIncrease, csize_t maxIter);
    virtual ~IterativeSolverBase ();

    const DDAParams<ftype>& ddaParams () const { return ddaParams_; }
    const DDAParams<ftype>& g () const { return ddaParams_; }
    const DipoleGeometry& dipoleGeometry () const { return ddaParams ().dipoleGeometry (); }

    boost::shared_ptr<std::vector<ctype> > getPolVec (const std::vector<ctype>& einc, ftype eps, std::ostream& log, const std::vector<ctype>& start = std::vector<ctype> (), Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
    void profilingRun (std::ostream& out, std::ostream& log, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());

    virtual void setCoupleConstants (const boost::shared_ptr<const CoupleConstants<ftype> >& cc) = 0;

  protected:
    ftype inprodR () const {
      return inprodR_;
    }
    ftype residScale;
    ftype epsB;

    virtual ftype initGeneral (const std::vector<ctype>& einc, std::ostream& log, const std::vector<ctype>& start, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) = 0;
    virtual void init (std::ostream& log, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) = 0;
    virtual ftype iteration (csize_t nr, std::ostream& log, bool profilingRun, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) = 0;
    virtual boost::shared_ptr<std::vector<ctype> > getResult (std::ostream& log, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) = 0;
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, IterativeSolverBase)
}

#endif // !DDA_ITERATIVESOLVERBASE_HPP_INCLUDED
