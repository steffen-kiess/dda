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

#ifndef MATH_DIAGMATRIX3_HPP_INCLUDED
#define MATH_DIAGMATRIX3_HPP_INCLUDED

// Math::DiagMatrix3<T> is a 3x3 matrix where all non-diagonal entries are zero.
//
// The difference to Math::Vector3<T> is mostly the effect of operations
// (Vector3<T> * Vector3<T> is a dot product while DiagMatrix3<T> * Vector3<T>
// is a matrix-vector-multiplication / an elementwise multiplication)

#include <Core/Assert.hpp>
#include <Core/Util.hpp>

#include <Math/Forward.hpp>
#include <Math/DiagMatrix3.hpp>

#include <complex>

#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>

namespace Math {
  template <typename T> class DiagMatrix3 {
    Vector3<T> diag_;

    class PrivateType {
      friend class DiagMatrix3;
      PrivateType () {}
    };

  public:
    DiagMatrix3 () {}
    explicit DiagMatrix3 (T v) : diag_ (v, v, v) {}
    DiagMatrix3 (T x, T y, T z) : diag_ (x, y, z) {}
    template <typename U> explicit DiagMatrix3 (const Vector3<U>& diag) : diag_ (diag) {}
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Wconversion"
    template <typename U> DiagMatrix3 (DiagMatrix3<U> v, UNUSED typename boost::enable_if<boost::is_convertible<U, T>, PrivateType>::type dummy = PrivateType ()) : diag_ (v.diag ()) {}
    template <typename U> explicit DiagMatrix3 (DiagMatrix3<U> v, UNUSED typename boost::disable_if<boost::is_convertible<U, T>, PrivateType>::type dummy = PrivateType ()) : diag_ (v.diag ()) {}
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic pop
#endif

    const Vector3<T>& diag () const {
      return diag_;
    }
    Vector3<T>& diag () {
      return diag_;
    }

    T m11 () const {
      return diag ().x ();
    }
    T m22 () const {
      return diag ().y ();
    }
    T m33 () const {
      return diag ().z ();
    }

    T& m11 () {
      return diag ().x ();
    }
    T& m22 () {
      return diag ().y ();
    }
    T& m33 () {
      return diag ().z ();
    }

    // off-diagonal entries (read-only, = 0)
    T m12 () const { return T (); }
    T m13 () const { return T (); }
    T m21 () const { return T (); }
    T m23 () const { return T (); }
    T m31 () const { return T (); }
    T m32 () const { return T (); }

    // Return the inverse matrix
    DiagMatrix3<T> inverse () const {
      return DiagMatrix3<T> (T (1) / m11 (), T (1) / m22 (), T (1) / m33 ());
    }

    // Access values on the diagonal
    const T& operator[] (size_t i) const {
      return diag ()[i];
    }
    T& operator[] (size_t i) {
      return diag ()[i];
    }
  };

  // Operations on DiagMatrix3

#define RTS(op) __decltype ((*(T*)NULL) op (*(U*)NULL))
#define RT(op) DiagMatrix3<RTS(op)>
  template <typename T, typename U> inline RT(+) operator+ (DiagMatrix3<T> v1, DiagMatrix3<U> v2) {
    return RT(+) (v1.m11 () + v2.m11 (), v1.m22 () + v2.m22 (), v1.m33 () + v2.m33 ());
  }

  template <typename T, typename U> inline DiagMatrix3<T>& operator+= (DiagMatrix3<T>& v1, DiagMatrix3<U> v2) {
    v1.m11 () += v2.m11 (); v1.m22 () += v2.m22 (); v1.m33 () += v2.m33 ();
    return v1;
  }


  template <typename T, typename U> inline RT(-) operator- (DiagMatrix3<T> v1, DiagMatrix3<U> v2) {
    return RT(-) (v1.m11 () - v2.m11 (), v1.m22 () - v2.m22 (), v1.m33 () - v2.m33 ());
  }

  template <typename T, typename U> inline DiagMatrix3<T>& operator-= (DiagMatrix3<T>& v1, DiagMatrix3<U> v2) {
    v1.m11 () -= v2.m11 (); v1.m22 () -= v2.m22 (); v1.m33 () -= v2.m33 ();
    return v1;
  }


  template <typename T, typename U> inline RT(*) operator* (DiagMatrix3<T> v, U scalar) {
    return RT(*) (v.m11 () * scalar, v.m22 () * scalar, v.m33 () * scalar);
  }

  template <typename T, typename U> inline RT(*) operator* (T scalar, DiagMatrix3<U> v) {
    return RT(*) (scalar * v.m11 (), scalar * v.m22 (), scalar * v.m33 ());
  }

  template <typename T, typename U> inline DiagMatrix3<T>& operator*= (DiagMatrix3<T>& v, U scalar) {
    v.m11 () *= scalar; v.m22 () *= scalar; v.m33 () *= scalar;
    return v;
  }

  template <typename T, typename U> inline RT(/) operator/ (DiagMatrix3<T> v, U scalar) {
    return RT(/) (v.m11 () / scalar, v.m22 () / scalar, v.m33 () / scalar);
  }

  template <typename T, typename U> inline DiagMatrix3<T>& operator/= (DiagMatrix3<T>& v, U scalar) {
    v.m11 () /= scalar; v.m22 () /= scalar; v.m33 () /= scalar;
    return v;
  }

  template <typename T> inline bool operator== (DiagMatrix3<T> v1, DiagMatrix3<T> v2) {
    return v1.m11 () == v2.m11 () && v1.m22 () == v2.m22 () && v1.m33 () == v2.m33 ();
  }
  template <typename T> inline bool operator!= (DiagMatrix3<T> v1, DiagMatrix3<T> v2) {
    return !(v1 == v2);
  }

  /*
  template <typename T> struct Abs2Impl<DiagMatrix3<T> > {
    static __decltype(Math::abs2 (*(T*)0)) apply (DiagMatrix3<T> v) {
      return Math::abs2 (v.m11 ()) + Math::abs2 (v.m22 ()) + Math::abs2 (v.m33 ());
    }
  };
  */

  // Matrix-Matrix multiplication
  template <typename T, typename U> inline RT(*) operator* (DiagMatrix3<T> v1, DiagMatrix3<U> v2) {
    return RT(*) (v1.m11 () * v2.m11 (), v1.m22 () * v2.m22 (), v1.m33 () * v2.m33 ());
  }

  // Matrix-Vector multiplication
  template <typename T, typename U> inline Vector3<RTS(*)> operator* (DiagMatrix3<T> v1, Vector3<U> v2) {
    return Vector3<RTS(*)> (v1.m11 () * v2.x (), v1.m22 () * v2.y (), v1.m33 () * v2.z ());
  }

#undef RTS
#undef RT

  // Unary +/-
  template <typename T> inline DiagMatrix3<T> operator+ (DiagMatrix3<T> v) {
    return DiagMatrix3<T> (+v.m11 (), +v.m22 (), +v.m33 ());
  }
  template <typename T> inline DiagMatrix3<T> operator- (DiagMatrix3<T> v) {
    return DiagMatrix3<T> (-v.m11 (), -v.m22 (), -v.m33 ());
  }

  template <typename F> DiagMatrix3<F> real (DiagMatrix3<std::complex<F> > v) {
    return DiagMatrix3<F> (real (v.m11 ()), real (v.m22 ()), real (v.m33 ()));
  }

  template <typename F> DiagMatrix3<F> imag (DiagMatrix3<std::complex<F> > v) {
    return DiagMatrix3<F> (imag (v.m11 ()), imag (v.m22 ()), imag (v.m33 ()));
  }
}

#endif // !MATH_DIAGMATRIX3_HPP_INCLUDED
