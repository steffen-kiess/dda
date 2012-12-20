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

#ifndef DDA_DIPOLEGEOMETRY_HPP_INCLUDED
#define DDA_DIPOLEGEOMETRY_HPP_INCLUDED

// A geometry given as a list of dipoles

#include <Core/OStream.hpp>
#include <Core/CheckedIntegerAlias.hpp>

#include <Math/Float.hpp>
#include <Math/Vector3.hpp>

#include <EMSim/Rotation.hpp>

#include <DDA/FPConst.hpp>
#include <DDA/Forward.hpp>

#include <cmath>
#include <iomanip>
#include <sstream>
#include <complex>
#include <algorithm>

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

namespace DDA {
  class DipoleGeometry {
    Math::Vector3<cuint32_t> box_; // Size of box around dipoles
    cuint8_t matCount_; // Maximum index in materialIndices + 1
    cuint32_t validNvCount_; // Number of non-void dipoles with valid bit set
    Math::Vector3<ldouble> origin_; // Position of dipole at integer position (0,0,0) in particle reference frame

    EMSim::Rotation<ldouble> orientation_; // Transformation particle ref => laboratory ref
    EMSim::Rotation<ldouble> orientationInverse_; // Transformation laboratory ref => particle ref
    ldouble gridUnit_; // Distance between two dipoles
    std::vector<Math::DiagMatrix3<cldouble> > materials_;
    Math::Vector3<ldouble> periodicity1_;
    Math::Vector3<ldouble> periodicity2_;

    ldouble gridUnitVol_;
    int periodicityDimension_;

    std::vector<Math::Vector3<uint32_t> > positions_; // List of positions of dipoles
    std::vector<uint8_t> materialIndices_; // List of material indicies pointing into "materials"
    std::vector<uint8_t> valid_; // List of boolean indicating whether the dipole should be considered for far field calculation etc.

  public:
    DipoleGeometry (ldouble gridUnit,
                    Math::Vector3<ldouble> periodicity1 = Math::Vector3<ldouble> (0, 0, 0),
                    Math::Vector3<ldouble> periodicity2 = Math::Vector3<ldouble> (0, 0, 0));

    Math::Vector3<cuint32_t> box () const {
      return box_;
    }

    Math::Vector3<ldouble> origin () const {
      return origin_;
    }
    Math::Vector3<ldouble>& origin () {
      return origin_;
    }

    cuint32_t matCount () const {
      return matCount_;
    }

    cuint32_t nvCount () const {
      return positions_.size ();
    }

    cuint32_t validNvCount () const {
      return validNvCount_;
    }

    const std::vector<Math::Vector3<uint32_t> >& positions () const {
      return positions_;
    }
    Math::Vector3<uint32_t> getGridCoordinates (size_t index) const {
      ASSERT (index < positions ().size ());
      return positions ()[index];
    }
    // Get the coordinates of dipole index in the particle reference frame
    template <typename ftype>
    Math::Vector3<ftype> getDipoleCoordPartRef (uint32_t index) const {
      Math::Vector3<uint32_t> gridCoord = getGridCoordinates (index);
      return static_cast<Math::Vector3<ftype> > (origin ()) + Math::Vector3<ftype> (gridCoord) * gridUnit ();
    }
    Math::Vector3<ldouble> getDipoleCoordPartRef (uint32_t index) const {
      return getDipoleCoordPartRef<ldouble> (index);
    }

    const std::vector<uint8_t>& materialIndices () const {
      return materialIndices_;
    }
    uint8_t getMaterialIndex (size_t index) const {
      ASSERT (index < materialIndices ().size ());
      return materialIndices ()[index];
    }

    const std::vector<uint8_t>& valid () const {
      return valid_;
    }
    bool isValid (size_t index) const {
      ASSERT (index < valid ().size ());
      return valid ()[index];
    }

    // Move the center of the particle to the center of the coordinate system
    void moveToCenter ();

    // Reset everything except gridUnit and periodicity
    void clear ();

    void addDipole (uint32_t x, uint32_t y, uint32_t z, uint8_t material, bool isValid = true);

    void normalize ();

    void check () const;

    void dump (bool verbose = false, const Core::OStream& out = Core::OStream::getStdout ()) const;

    const EMSim::Rotation<ldouble>& orientation () const { return orientation_; }
    const EMSim::Rotation<ldouble>& orientationInverse () const { return orientationInverse_; }
    void orientation (const EMSim::Rotation<ldouble>& value);

    ldouble gridUnit () const { return gridUnit_; }
    const std::vector<Math::DiagMatrix3<cldouble> >& materials () const { return materials_; }
    std::vector<Math::DiagMatrix3<cldouble> >& materials () { return materials_; }
    Math::Vector3<ldouble> periodicity1 () const { return periodicity1_; }
    Math::Vector3<ldouble> periodicity2 () const { return periodicity2_; }

    ldouble gridUnitVol () const { return gridUnitVol_; }
    int periodicityDimension () const { return periodicityDimension_; }
  };
}

#endif // !DDA_DIPOLEGEOMETRY_HPP_INCLUDED
