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

#include "Shapes.hpp"

#include <Core/ParsingUtil.hpp>

#include <Math/Vector3IOS.hpp>
#include <Math/DiagMatrix3.hpp>

#include <EMSim/Parse.hpp>

#include <DDA/DipoleGeometry.hpp>
#include <DDA/GeometryParser.hpp>

#include <cstring>
#include <cstdlib>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/make_shared.hpp>

namespace DDA {
  namespace Shapes {
    boost::shared_ptr<Sphere> Sphere::parse (const std::string& s) {
      std::vector<std::string> strs;
      boost::split (strs, s, boost::is_any_of (","));
      ASSERT (strs.size () == 2);
      EMSim::Length radius = boost::lexical_cast<EMSim::Length> (strs[0]);
      Math::DiagMatrix3<cldouble> material;
      EMSim::parse (strs[1], material);
      return boost::make_shared<Sphere> (radius, material);
    }
    void Sphere::createDipoleGeometry (DipoleGeometry& dipoleGeometry) const {
      double radius = static_cast<double> (this->radius () / dipoleGeometry.gridUnit ());
      double diameter = radius * 2;
      double r2 = radius * radius;
      size_t mat = dipoleGeometry.materials ().size ();
      dipoleGeometry.materials ().push_back (material ());
      for (uint32_t z = 0; z < diameter; z++) {
        for (uint32_t y = 0; y < diameter; y++) {
          for (uint32_t x = 0; x < diameter; x++) {
            double xd = x - radius + 0.5;
            double yd = y - radius + 0.5;
            double zd = z - radius + 0.5;
            if (xd * xd + yd * yd + zd * zd <= r2) {
              ASSERT (mat <= 255);
              dipoleGeometry.addDipole (x, y, z, static_cast<uint8_t> (mat));
            }
          }
        }
      }
      dipoleGeometry.moveToCenter ();
    }
    Sphere::~Sphere () {}
    void Sphere::getDimensionsMaterials (std::vector<ldouble>& dimensions, std::vector<Math::DiagMatrix3<cldouble> >& materials) const {
      dimensions.push_back (radius ().value () * 2);
      materials.push_back (material ());
    }
    GEOMETRYPARSER_CREATE_IMPL (sphereParser, Sphere, "sphere", "<radius>,<material>")

    boost::shared_ptr<Cylinder> Cylinder::parse (const std::string& s) {
      std::vector<std::string> strs;
      boost::split (strs, s, boost::is_any_of (","));
      ASSERT (strs.size () == 3);
      EMSim::Length length = boost::lexical_cast<EMSim::Length> (strs[0]);
      EMSim::Length radius = boost::lexical_cast<EMSim::Length> (strs[1]);
      Math::DiagMatrix3<cldouble> material;
      EMSim::parse (strs[2], material);
      return boost::make_shared<Cylinder> (length, radius, material);
    }
    void Cylinder::createDipoleGeometry (DipoleGeometry& dipoleGeometry) const {
      double length = static_cast<double> (this->length () / dipoleGeometry.gridUnit ());
      double radius = static_cast<double> (this->radius () / dipoleGeometry.gridUnit ());
      double diameter = radius * 2;
      double r2 = radius * radius;
      size_t mat = dipoleGeometry.materials ().size ();
      dipoleGeometry.materials ().push_back (material ());
      for (uint32_t z = 0; z < diameter; z++) {
        for (uint32_t y = 0; y < diameter; y++) {
          double yd = y - radius + 0.5;
          double zd = z - radius + 0.5;
          if (yd * yd + zd * zd <= r2) {
            for (uint32_t x = 0; x < length; x++) {
              ASSERT (mat <= 255);
              dipoleGeometry.addDipole (x, y, z, static_cast<uint8_t> (mat));
            }
          }
        }
      }
      dipoleGeometry.moveToCenter ();
    }
    Cylinder::~Cylinder () {}
    void Cylinder::getDimensionsMaterials (std::vector<ldouble>& dimensions, std::vector<Math::DiagMatrix3<cldouble> >& materials) const {
      dimensions.push_back (length ().value ());
      dimensions.push_back (radius ().value () * 2);
      materials.push_back (material ());
    }
    GEOMETRYPARSER_CREATE_IMPL (cylinderParser, Cylinder, "cylinder", "<length>,<radius>,<material>")

    boost::shared_ptr<Box> Box::parse (const std::string& s) {
      std::vector<std::string> strs;
      boost::split (strs, s, boost::is_any_of (","));
      size_t pos = s.rfind (',');
      ASSERT (pos != std::string::npos);
      Math::Vector3<EMSim::Length> l = boost::lexical_cast<Math::Vector3<EMSim::Length> > (s.substr (0, pos));
      Math::DiagMatrix3<cldouble> material;
      EMSim::parse (s.substr (pos + 1), material);
      return boost::make_shared<Box> (l, material);
    }
    void Box::createDipoleGeometry (DipoleGeometry& dipoleGeometry) const {
      Math::Vector3<ldouble> size = this->size () / dipoleGeometry.gridUnit ();
      size_t mat = dipoleGeometry.materials ().size ();
      dipoleGeometry.materials ().push_back (material ());
      for (uint32_t z = 0; z < size.z (); z++) {
        for (uint32_t y = 0; y < size.y (); y++) {
          for (uint32_t x = 0; x < size.x (); x++) {
            ASSERT (mat <= 255);
            dipoleGeometry.addDipole (x, y, z, static_cast<uint8_t> (mat));
          }
        }
      }
      dipoleGeometry.moveToCenter ();
    }
    Box::~Box () {}
    void Box::getDimensionsMaterials (std::vector<ldouble>& dimensions, std::vector<Math::DiagMatrix3<cldouble> >& materials) const {
      dimensions.push_back (size ().x ().value ());
      dimensions.push_back (size ().y ().value ());
      dimensions.push_back (size ().z ().value ());
      materials.push_back (material ());
    }
    GEOMETRYPARSER_CREATE_IMPL (boxParser, Box, "box", "(<x>,<y>,<z>),<material>")
  }
}
