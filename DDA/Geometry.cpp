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

#include "Geometry.hpp"

#include <DDA/DipoleGeometry.hpp>

#include <boost/make_shared.hpp>

namespace DDA {
  Geometry::Geometry ()
    : orientation_ (Math::Quaternion<ldouble> (1, 0, 0, 0)),
      periodicity1_ (0, 0, 0),
      periodicity2_ (0, 0, 0),
      periodicityDimension_ (0)
  {
  }

  Geometry::~Geometry () {}

#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Wfloat-equal"
  void Geometry::setPeriodicity (Math::Vector3<ldouble> periodicity1,
                                 Math::Vector3<ldouble> periodicity2) {
    bool periodicity1IsZero = periodicity1.x () == 0 && periodicity1.y () == 0 && periodicity1.z () == 0;
    bool periodicity2IsZero = periodicity2.x () == 0 && periodicity2.y () == 0 && periodicity2.z () == 0;
    if (periodicity1IsZero) {
      ASSERT (periodicity2IsZero);
      periodicityDimension_ = 0;
    } else {
      if (periodicity2IsZero) {
        periodicityDimension_ = 1;
      } else {
        periodicityDimension_ = 2;
      }
    }

    periodicity1_ = periodicity1;
    periodicity2_ = periodicity2;
  }
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic pop
#endif

  boost::shared_ptr<DipoleGeometry> Geometry::createDipoleGeometry (ldouble gridUnit) const {
    boost::shared_ptr<DipoleGeometry> ptr = boost::make_shared<DipoleGeometry> (gridUnit, periodicity1 (), periodicity2 ());
    ptr->orientation (orientation ());
    createDipoleGeometry (*ptr);
    return ptr;
  }
}
