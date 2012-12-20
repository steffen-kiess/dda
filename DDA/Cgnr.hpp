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

#ifndef DDA_CGNR_HPP_INCLUDED
#define DDA_CGNR_HPP_INCLUDED

// CGNR solver on the CPU

#include <DDA/CpuIterativeSolver.hpp>

namespace DDA {
  template <typename T>
  class Cgnr : public CpuIterativeSolver<T> {
    typedef T ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

    struct VarType {
      ctype beta;
      ftype ro_new;
      ftype ro_old;
    } vars;

    using CpuIterativeSolver<T>::g;

  public:
    Cgnr (const DDAParams<ftype>& ddaParams, MatVec<ftype>& matVec, csize_t maxIter);
    virtual ~Cgnr ();

    virtual void init (std::ostream& log, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
    virtual ftype iteration (csize_t nr, std::ostream& log, bool profilingRun, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, Cgnr)
}

#endif // !DDA_CGNR_HPP_INCLUDED
