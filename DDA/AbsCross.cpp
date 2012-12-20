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

#include "AbsCross.hpp"

namespace DDA {
  // Absorption cross-section, Draine
  template <class ftype>
  ftype AbsCross<ftype>::absCross (const DDAParams<ftype>& ddaParams, const std::vector<ctype>& pvec, const CoupleConstants<ftype>& cc) {
    ftype temp1 = std::pow (ddaParams.waveNum (), FPConst<ftype>::three) * 2 / 3;

    std::vector<Math::DiagMatrix3<ftype> > multDr (ddaParams.dipoleGeometry ().matCount () ());
    for (size_t i = 0; i < ddaParams.dipoleGeometry ().matCount (); i++)
      for (int j = 0; j < 3; j++)
        multDr[i][j] = -imag (Const::one / cc.cc ()[i][j])-temp1;

    ftype sum = 0;

    for (uint32_t i = 0; i < ddaParams.cnvCount (); i++) {
      if (ddaParams.dipoleGeometry ().isValid (i)) {
        uint8_t mat = ddaParams.dipoleGeometry ().getMaterialIndex (i);
        for (int j = 0; j < 3; j++) {
          sum += multDr[mat][j] * norm (pvec[i + j * ddaParams.vecStride ()]);
        }
      }
    }

    return Const::four_pi * ddaParams.waveNum () * sum;
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, AbsCross)
}
