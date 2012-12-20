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

#ifndef DDA_MATVEC_HPP_INCLUDED
#define DDA_MATVEC_HPP_INCLUDED

// Base class for the matrix-vector-product

#include <Core/Profiling.hpp>

#include <DDA/DDAParams.hpp>

namespace DDA {
  template <typename T>
  class MatVec {
    typedef T ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

    const DDAParams<ftype>& ddaParams_;
    boost::shared_ptr<const CoupleConstants<ftype> > cc_;

  public:
    MatVec (const DDAParams<ftype>& ddaParams);
    virtual ~MatVec ();

    virtual void setCoupleConstants (const boost::shared_ptr<const CoupleConstants<ftype> >& cc);

    const DDAParams<ftype>& ddaParams () const { return ddaParams_; }
    const DDAParams<ftype>& g () const { return ddaParams_; }
    const Geometry& geometry () const { return ddaParams ().geometry (); }
    const boost::shared_ptr<const CoupleConstants<ftype> >& ccPtr () const { return cc_; }
    const CoupleConstants<ftype>& cc () const { return *ccPtr (); }

    virtual void apply (const std::vector<ctype>& arg, std::vector<ctype>& result, bool conj, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) = 0;
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, MatVec)
}

#endif // !DDA_MATVEC_HPP_INCLUDED
