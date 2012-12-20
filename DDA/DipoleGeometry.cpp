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

#include "DipoleGeometry.hpp"

#include <Core/OStream.hpp>
#include <Core/IStream.hpp>

#include <Math/DiagMatrix3.hpp>
#include <Math/DiagMatrix3IOS.hpp>

#include <DDA/Geometry.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include <cstring>

namespace DDA {
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Wfloat-equal"
  DipoleGeometry::DipoleGeometry (ldouble gridUnit,
                                  Math::Vector3<ldouble> periodicity1,
                                  Math::Vector3<ldouble> periodicity2)
    : box_ (0, 0, 0), matCount_ (0), validNvCount_ (0), origin_ (0, 0, 0),
      orientation_ (EMSim::Rotation<ldouble>::none ()),
      orientationInverse_ (EMSim::Rotation<ldouble>::none ()),
      gridUnit_ (gridUnit),
      periodicity1_ (periodicity1),
      periodicity2_ (periodicity2)
  {
    ASSERT (gridUnit >= 0);

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

    gridUnitVol_ = gridUnit * gridUnit * gridUnit;
  }
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic pop
#endif


  void DipoleGeometry::moveToCenter () {
    if (box_ != Math::Vector3<cuint32_t> (0, 0, 0))
      origin_ = -Math::Vector3<ldouble> ((box_.x () () - 1.0l) / 2, (box_.y () () - 1.0l) / 2, (box_.z () () - 1.0l) / 2) * gridUnit ();
    else
      origin_ = Math::Vector3<ldouble> (0, 0, 0);
  }

  void DipoleGeometry::clear () {
    box_ = Math::Vector3<cuint32_t> (0, 0, 0);
    matCount_ = 0;
    positions_.clear ();
    materialIndices_.clear ();
    origin_ = Math::Vector3<ldouble> (0, 0, 0);
  }

  void DipoleGeometry::addDipole (uint32_t x, uint32_t y, uint32_t z, uint8_t material, bool isValid) {
    ASSERT (box_.z () <= cuint32_t (z) + 1);
    positions_.push_back (Math::Vector3<uint32_t> (x, y, z));
    materialIndices_.push_back (material);
    valid_.push_back (isValid);
    if (isValid)
      validNvCount_++;
    if (x >= box_.x () || y >= box_.y () || z >= box_.z ()) {
      if (x >= box_.x ())
        box_.x () = cuint32_t (x) + 1;
      if (y >= box_.y ())
        box_.y () = cuint32_t (y) + 1;
      if (z >= box_.z ())
        box_.z () = cuint32_t (z) + 1;
    }
    if (material >= matCount_)
      matCount_ = cuint8_t (material) + 1;
  }

  void DipoleGeometry::normalize () {
    if (nvCount () == 0)
      return;

    Math::Vector3<uint32_t> min = Math::Vector3<uint32_t> (std::numeric_limits<uint32_t>::max (), std::numeric_limits<uint32_t>::max (), std::numeric_limits<uint32_t>::max ());

    for (uint32_t i = 0; i < nvCount (); i++) {
      Math::Vector3<uint32_t> coords = getGridCoordinates (i);
      if (coords.x () < min.x ())
        min.x () = coords.x ();
      if (coords.y () < min.y ())
        min.y () = coords.y ();
      if (coords.z () < min.z ())
        min.z () = coords.z ();
    }

    if (min != Math::Vector3<uint32_t> (0, 0, 0)) {
      for (uint32_t i = 0; i < nvCount (); i++) {
        positions_[i] -= min;
      }
      box_ -= min;
      origin () += min * gridUnit ();
    }
  }

  void DipoleGeometry::check () const {
    ASSERT (nvCount () == positions_.size ());
    ASSERT (nvCount () == materialIndices_.size ());
  }

  void DipoleGeometry::dump (bool verbose, const Core::OStream& out) const {
    if (verbose)
      for (uint32_t i = 0; i < nvCount (); i++)
        out << i << " " << getGridCoordinates (i) << " " << (int) getMaterialIndex (i) << std::endl;
    out << "box = " << box () << std::endl;
    out << "matCount = " << matCount () << std::endl;
    out << "nvCount = " << nvCount () << std::endl;
    check ();
  }

  void DipoleGeometry::orientation (const EMSim::Rotation<ldouble>& value) {
    orientation_ = value;
    orientationInverse_ = value.inverse ();
  }
}
