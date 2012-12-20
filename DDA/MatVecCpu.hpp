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

#ifndef DDA_MATVECCPU_HPP_INCLUDED
#define DDA_MATVECCPU_HPP_INCLUDED

// matrix-vector-product implementation for the CPU

#include <Core/Profiling.hpp>
#include <Core/Allocator.hpp>

#include <DDA/MatVec.hpp>

namespace DDA {
  template <typename T>
  class MatVecCpu : public MatVec<T> {
    typedef T ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

    boost::const_multi_array_ref<Math::SymMatrix3<ctype>, 3> Dmatrix_;

#define MAX(x, y) ((x) > (y) ? (x) : (y))
    typedef Core::Allocator<ctype, MAX (boost::alignment_of<ctype>::value, 16)> Allocator;
#undef MAX

    // Data structures and FFT plans for matVec
    boost::multi_array<ctype, 4, Allocator> Xmatrix;
    boost::multi_array<ctype, 3, Allocator> slices;
    boost::multi_array<ctype, 3, Allocator> slices_tr;
    // times = Xmatrix.sizeZ () * 3, stride = Xmatrix.sizeY * Xmatrix.sizeX
    boost::shared_ptr<LinAlg::FFTPlan<ftype> >  planX;
    // times = 3, stride = gridY * gridZ
    boost::shared_ptr<LinAlg::FFTPlan<ftype> >  planZ;
    // times = 1
    boost::shared_ptr<LinAlg::FFTPlan<ftype> >  planY;

    const boost::const_multi_array_ref<Math::SymMatrix3<ctype>, 3>& dMatrix () const { return Dmatrix_; }

  public:
    MatVecCpu (const DDAParams<ftype>& ddaParams, const boost::const_multi_array_ref<Math::SymMatrix3<ctype>, 3>& Dmatrix, const LinAlg::FFTPlanFactory<ftype>& planFactory);
    virtual ~MatVecCpu ();

    const DDAParams<ftype>& ddaParams () const { return MatVec<T>::ddaParams(); }
    const DDAParams<ftype>& g () const { return MatVec<T>::ddaParams(); }
    const DipoleGeometry& dipoleGeometry () const { return ddaParams ().dipoleGeometry (); }

    virtual void apply (const std::vector<ctype>& arg, std::vector<ctype>& result, bool conj, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, MatVecCpu)
}

#endif // !DDA_MATVECCPU_HPP_INCLUDED
