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

#ifndef DDA_DMATRIXCPU_HPP_INCLUDED
#define DDA_DMATRIXCPU_HPP_INCLUDED

// DMatrix calculation on the CPU

#include <DDA/DDAParams.hpp>

namespace DDA {
  template <class T>
  class DMatrixCpu {
    typedef T ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

  private:
    static inline ctype getInteractionTerm (const DDAParams<T>& ddaParams, int i, int j, int k, int mu, int nu, const boost::shared_ptr<const Beam<T> >& beam);
    static inline uint32_t clip (int32_t n, uint32_t M);

  public:
    static void createDMatrix (const DDAParams<T>& ddaParams, const LinAlg::FFTPlanFactory<T>& planFactory, boost::multi_array_ref<Math::SymMatrix3<std::complex<T> >, 3>& dMatrix, const boost::shared_ptr<const Beam<T> >& beam);
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, DMatrixCpu)
}

#endif // !DDA_DMATRIXCPU_HPP_INCLUDED
