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

#ifndef DDA_GEOMETRY_HPP_INCLUDED
#define DDA_GEOMETRY_HPP_INCLUDED

// Abstract base class for geometries (implemented by different shapes)

#include <Core/CheckedInteger.hpp>

#include <Math/DiagMatrix3.hpp>
#include <Math/Float.hpp>

#include <EMSim/Rotation.hpp>
#include <EMSim/Length.hpp>

#include <DDA/Forward.hpp>

#include <vector>

#include <boost/shared_ptr.hpp>

namespace DDA {
  class Geometry {
    EMSim::Rotation<ldouble> orientation_; // Transformation particle ref => laboratory ref
    Math::Vector3<ldouble> periodicity1_;
    Math::Vector3<ldouble> periodicity2_;

    int periodicityDimension_;

  public:
    Geometry ();
    virtual ~Geometry ();

    virtual void createDipoleGeometry (DipoleGeometry& dipoleGeometry) const = 0;
    // true = dipole model is symmatric with respect to rotation by 90 degrees over the z-axis
    virtual bool isSymmetric () const { return false; }

    boost::shared_ptr<DipoleGeometry> createDipoleGeometry (ldouble gridUnit) const;

    virtual void getDimensionsMaterials (std::vector<ldouble>& dimensions, std::vector<Math::DiagMatrix3<cldouble> >& materials) const = 0;

    const EMSim::Rotation<ldouble>& orientation () const { return orientation_; }
    const Math::Vector3<ldouble>& periodicity1 () const { return periodicity1_; }
    const Math::Vector3<ldouble>& periodicity2 () const { return periodicity2_; }
    int periodicityDimension () const { return periodicityDimension_; }

    EMSim::Rotation<ldouble>& orientation () { return orientation_; }
    void setPeriodicity (Math::Vector3<ldouble> periodicity1 = Math::Vector3<ldouble> (0, 0, 0),
                         Math::Vector3<ldouble> periodicity2 = Math::Vector3<ldouble> (0, 0, 0));
  };
}

#endif // !DDA_GEOMETRY_HPP_INCLUDED
