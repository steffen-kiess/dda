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

#ifndef EMSIM_ROTATION_HPP_INCLUDED
#define EMSIM_ROTATION_HPP_INCLUDED

// Class representing a rotation in 3d space using a quaternion

#include <Core/Assert.hpp>

#include <Math/Vector3.hpp>
#include <Math/Quaternion.hpp>
#include <Math/QuaternionIOS.hpp>
#include <Math/FPTemplateInstances.hpp>

#include <cmath>
#include <ostream>

#include <boost/lexical_cast.hpp>
#include <boost/math/constants/constants.hpp>

namespace EMSim {
  template <typename ftype> class Rotation {
#ifdef SWIG_VISIBILITY_WORKAROUND
  public:
#endif
    typedef Math::Quaternion<ftype> Q;
    typedef Rotation<ftype> R;
    typedef Math::Vector3<ftype> V;
  private:

    Q q;

  public:
    static R none () {
      return R (Q (1, 0, 0, 0));
    }

    Rotation (Q value) : q (value) {
      ftype len = Math::abs (q);
      ASSERT_MSG (len >= 0.99 && len <= 1.01, boost::lexical_cast<std::string> (len));
      q /= len;
    }

    const Q& quaternion () const {
      return q;
    }

    V operator* (V v) const {
      Q res = q * Q (0, v.x (), v.y (), v.z ()) * q.conjugate ();
      return V (res.b (), res.c (), res.d ());
    }

    R operator* (R r) const {
      return Rotation (q * r.q);
    }
    R& operator*= (R r) {
      *this = *this * r;
      return *this;
    }

    R inverse () const {
      return R (q.conjugate ());
    }

    static R fromAxisAngle (V axis, ftype rad) {
      ftype sin = std::sin (rad / 2);
      ftype cos = std::cos (rad / 2);
      return R (Q (cos, axis.x () * sin, axis.y () * sin, axis.z () * sin));
    }
    static R fromAxisAngleDeg (V axis, ftype rad) {
      return fromAxisAngle (axis, rad / 180 * boost::math::constants::pi<ftype> ());
    }
    static R fromZYZ (ftype alpha, ftype beta, ftype gamma) {
      return fromAxisAngle (V (0, 0, 1), alpha)
        * fromAxisAngle (V (0, 1, 0), beta) 
        * fromAxisAngle (V (0, 0, 1), gamma);
    }
    static R fromZYZDeg (ftype alpha, ftype beta, ftype gamma) {
      return fromZYZ (alpha / 180 * boost::math::constants::pi<ftype> (),
                      beta / 180 * boost::math::constants::pi<ftype> (),
                      gamma / 180 * boost::math::constants::pi<ftype> ());
    }

    void toZYZ (ftype& alpha, ftype& beta, ftype& gamma) const {
      // Some rotation matrix elements
      // (m11 m21 m31)
      // (m12 m22 m32)
      // (m13 m23 m33)
      ftype m33 = 1 - 2 * q.b () * q.b () - 2 * q.c () * q.c ();
      //Core::OStream::getStdout () << m33 << std::endl;
      ftype eps = static_cast<ftype> (1e-6l); // = 89.918435 deg
      if (m33 > 1 - eps) { // m33 = cos b ~ 1
        ftype m11 = 1 - 2 * q.c () * q.c () - 2 * q.d () * q.d (); // = cos a cos b cos g - sin a sin g = cos a cos g - sin a sin g = cos (a + g)
        ftype m12 = 2 * q.b () * q.c () + 2 * q.a () * q.d (); // = cos a sin g + sin a cos b cos g = cos a sin g + sin a cos g = sin (a + g)
        alpha = std::atan2 (m12, m11); // a + g
        beta = 0;
        gamma = 0;
      } else if (m33 < -1 + eps) { // m33 = cos b ~ -1
        ftype m11 = 1 - 2 * q.c () * q.c () - 2 * q.d () * q.d (); // = cos a cos b cos g - sin a sin g = - cos a cos g - sin a sin g = - cos (a - g)
        ftype m12 = 2 * q.b () * q.c () + 2 * q.a () * q.d (); // = cos a sin g + sin a cos b cos g = cos a sin g - sin a cos g = - sin (a - g)
        alpha = std::atan2 (-m12, -m11); // a - g
        beta = boost::math::constants::pi<ftype> ();
        gamma = 0;
      } else {
        ftype m13 = 2 * q.b () * q.d () - 2 * q.a () * q.c ();
        ftype m23 = 2 * q.c () * q.d () + 2 * q.a () * q.b ();
        ftype m31 = 2 * q.b () * q.d () + 2 * q.a () * q.c ();
        ftype m32 = 2 * q.c () * q.d () - 2 * q.a () * q.b ();
        alpha = std::atan2 (m32, m31);
        beta = std::acos (m33);
        gamma = std::atan2 (m23, -m13);
      }
    }
    void toZYZDeg (ftype& alpha, ftype& beta, ftype& gamma) const {
      toZYZ (alpha, beta, gamma);
      alpha *= 180 / boost::math::constants::pi<ftype> ();
      beta *= 180 / boost::math::constants::pi<ftype> ();
      gamma *= 180 / boost::math::constants::pi<ftype> ();
    }

    // Return the squared "difference" between two rotations, is always positive,
    // should be ~0 for this == r
    ftype squaredDifference (R r) const {
      Q diff = (r * inverse ()).quaternion (); // When this == r this should be an identical rotation
      if (diff.a () < 0) // Map (-1, 0, 0, 0) to (1, 0, 0, 0)
        diff *= -1;
      diff -= Q (1, 0, 0, 0); // Map to (0, 0, 0, 0)
      return Math::abs2 (diff);
    }
  };

  template <typename ftype>
  std::ostream& operator<< (std::ostream& stream, const Rotation<ftype>& r) {
    return stream << r.quaternion ();
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, Rotation)
}

#endif // !EMSIM_ROTATION_HPP_INCLUDED
