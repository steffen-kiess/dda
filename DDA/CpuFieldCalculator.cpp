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

#include "CpuFieldCalculator.hpp"

#include <Math/Vector3.hpp>

#include <DDA/DDAParams.hpp>

namespace DDA {
  template <class T>
  CpuFieldCalculator<T>::CpuFieldCalculator (const DDAParams<ftype>& ddaParams) : FieldCalculator<ftype> (ddaParams), pvec (0), xValues (ddaParams.dipoleGeometry ().box ().x () ()) {}

  template <class T>
  CpuFieldCalculator<T>::~CpuFieldCalculator () {}

  template <class T>
  void CpuFieldCalculator<T>::setPVec (const std::vector<ctype>& pvec) {
    ASSERT (pvec.size () == ddaParams ().cvecSize ());
    this->pvec = pvec;
  }

  template <class T>
  /* // This does not work because the functions in std::complex do not get -ffast-math
     #if GCC_VERSION_IS_ATLEAST (4, 4)
     __attribute__((__optimize__("fast-math")))
     #endif
  */
  Math::Vector3<std::complex<T> > CpuFieldCalculator<T>::calcField (Math::Vector3<ftype> n) {
    Math::Vector3<ctype> sum (0, 0, 0);

    uint32_t iy1 = 0; iy1--;
    uint32_t iz1 = 0; iz1--;

    ctype tmp, tmpZ;

    // Move some checks out of the loop
    uint32_t nvCount = ddaParams ().nvCount ();
    uint32_t boxX = ddaParams ().dipoleGeometry ().box ().x () ();
    ftype kd = ddaParams ().kd ();
    const std::vector<Math::Vector3<uint32_t> >& positions = ddaParams ().dipoleGeometry ().positions ();
    ASSERT (positions.size () == nvCount);
    const std::vector<uint8_t>& valid = ddaParams ().dipoleGeometry ().valid ();
    ASSERT (valid.size () == nvCount);

    //std::vector<ctype> xValues (boxX);
    ASSERT (this->xValues.size () == boxX);
    ctype* xValues = this->xValues.data ();
    for (uint32_t i = 0; i < boxX; i++) {
      xValues[i] = std::polar<ftype> (1, -kd * n.x () * static_cast <ftype> (i));
    }

    for (uint32_t j = 0; j < nvCount; j++) {
      //if (dipoleGeometry ().isValid (j)) {
      if (valid[j]) {
        //Math::Vector3<uint32_t> i = dipoleGeometry ().getGridCoordinates (j);
        Math::Vector3<uint32_t> i = positions[j];
        if (i.y () != iy1 || i.z () != iz1) {
          //Core::OStream::getStdout () << j << "y" << (i.y () != iy1) << "z" << (i.z () != iz1) << std::endl;
          if (i.z () != iz1) {
            iz1 = i.z ();
            tmpZ = std::polar<ftype> (1, -kd * n.z () * static_cast<ftype> (i.z ()));
          }
          iy1 = i.y ();
          tmp = std::polar<ftype> (1, -kd * n.y () * static_cast<ftype> (i.y ())) * tmpZ;
        }
        //ctype a = tmp * std::polar<ftype> (1, -kd * n.x () * static_cast <ftype> (i.x ()));
        ctype a = tmp * xValues[i.x ()];
        /*
        //ctype a = std::polar<ftype> (1, -kd * (n * Math::Vector3<ftype> (i)));
        ftype arg = -kd * (n * Math::Vector3<ftype> (i));
        ctype a = ctype (std::cos (arg), std::sin (arg));
        */
        sum += ddaParams ().get (pvec, j) * a;
      }
    }
    Math::Vector3<ctype> tbuff = sum - n * (n * sum);
    return (ctype (0, std::pow (ddaParams ().waveNum (), FPConst<ftype>::two)) * std::polar<ftype> (1, -ddaParams ().waveNum () * (static_cast<Math::Vector3<ftype> > (ddaParams ().dipoleGeometry ().origin ()) * n))) * tbuff;
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES (CREATE_TEMPLATE_INSTANCE, CpuFieldCalculator)
}
