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

#ifndef DDA_SHAPES_HPP_INCLUDED
#define DDA_SHAPES_HPP_INCLUDED

// Implementations of Geometry for spheres / cylinders / boxes

#include <Math/Float.hpp>

#include <DDA/Geometry.hpp>

namespace DDA {
  namespace Shapes {
    class Sphere : public Geometry {
      EMSim::Length radius_;
      Math::DiagMatrix3<cldouble> material_;

    public:
      static boost::shared_ptr<Sphere> parse (const std::string& s);

      Sphere (EMSim::Length radius, Math::DiagMatrix3<cldouble> material) : radius_ (radius), material_ (material) {}
      virtual ~Sphere ();

      EMSim::Length radius () const { return radius_; }
      const Math::DiagMatrix3<cldouble>& material () const { return material_; }

      virtual void createDipoleGeometry (DipoleGeometry& dipoleGeometry) const;
      virtual bool isSymmetric () const { return true; }

      virtual void getDimensionsMaterials (std::vector<ldouble>& dimensions, std::vector<Math::DiagMatrix3<cldouble> >& materials) const;
    };
    boost::shared_ptr<const GeometryParser> sphereParser ();

    class Cylinder : public Geometry {
      EMSim::Length length_;
      EMSim::Length radius_;
      Math::DiagMatrix3<cldouble> material_;

    public:
      static boost::shared_ptr<Cylinder> parse (const std::string& s);

      Cylinder (EMSim::Length length, EMSim::Length radius, Math::DiagMatrix3<cldouble> material) : length_ (length), radius_ (radius), material_ (material) {}
      virtual ~Cylinder ();

      EMSim::Length length () const { return length_; }
      EMSim::Length radius () const { return radius_; }
      const Math::DiagMatrix3<cldouble>& material () const { return material_; }

      virtual void createDipoleGeometry (DipoleGeometry& dipoleGeometry) const;

      virtual void getDimensionsMaterials (std::vector<ldouble>& dimensions, std::vector<Math::DiagMatrix3<cldouble> >& materials) const;
    };
    boost::shared_ptr<const GeometryParser> cylinderParser ();

    class Box : public Geometry {
      Math::Vector3<EMSim::Length> size_;
      Math::DiagMatrix3<cldouble> material_;

    public:
      static boost::shared_ptr<Box> parse (const std::string& s);

      Box (Math::Vector3<EMSim::Length> size, Math::DiagMatrix3<cldouble> material) : size_ (size), material_ (material) {}
      virtual ~Box ();

      const Math::Vector3<EMSim::Length>& size () const { return size_; }
      const Math::DiagMatrix3<cldouble>& material () const { return material_; }

      virtual void createDipoleGeometry (DipoleGeometry& dipoleGeometry) const;

      virtual void getDimensionsMaterials (std::vector<ldouble>& dimensions, std::vector<Math::DiagMatrix3<cldouble> >& materials) const;
    };
    boost::shared_ptr<const GeometryParser> boxParser ();
  }
}

#endif // !DDA_SHAPES_HPP_INCLUDED
