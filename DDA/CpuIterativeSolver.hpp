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

#ifndef DDA_CPUITERATIVESOLVER_HPP_INCLUDED
#define DDA_CPUITERATIVESOLVER_HPP_INCLUDED

// Base class for iterative solvers running on the CPU

#include <Core/Profiling.hpp>

#include <DDA/IterativeSolverBase.hpp>
#include <DDA/MatVec.hpp>

namespace DDA {
  template <typename T>
  class CpuIterativeSolver : public IterativeSolverBase<T> {
    typedef T ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

    MatVec<ftype>& matVec_;

    std::vector<ctype> Avecbuffer_;
    std::vector<ctype> rvec_;
    std::vector<ctype> xvec_;
    std::vector<ctype> tmpVec1_;

  public:
    CpuIterativeSolver (const DDAParams<ftype>& ddaParams, MatVec<ftype>& matVec, csize_t maxResIncrease, csize_t maxIter);
    virtual ~CpuIterativeSolver ();

    using IterativeSolverBase<T>::g;
    using IterativeSolverBase<T>::dipoleGeometry;
    MatVec<ftype>& matVec () { return matVec_; }

    virtual void setCoupleConstants (const boost::shared_ptr<const CoupleConstants<ftype> >& cc);

  protected:
    std::vector<ctype>& Avecbuffer () { return Avecbuffer_; }
    std::vector<ctype>& rvec () { return rvec_; }
    std::vector<ctype>& xvec () { return xvec_; }
    std::vector<ctype>& tmpVec1 () { return tmpVec1_; }

    virtual ftype initGeneral (const std::vector<ctype>& einc, std::ostream& log, const std::vector<ctype>& start, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
    virtual boost::shared_ptr<std::vector<ctype> > getResult (std::ostream& log, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, CpuIterativeSolver)
}

#endif // !DDA_CPUITERATIVESOLVER_HPP_INCLUDED
