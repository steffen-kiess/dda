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

#ifndef MATH_VECTOR3_HPP_INCLUDED
#define MATH_VECTOR3_HPP_INCLUDED

// Math::DiagMatrix3<T> is a 3-dimensional vector

#include <Core/Util.hpp>
#include <Core/Assert.hpp>

#include <Math/Forward.hpp>
#include <Math/Abs.hpp>

#include <complex>

#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/utility/enable_if.hpp>

//#define VECTOR3_USE_ARRAY
//#define VECTOR3_USE_PADDING

namespace Math {
  template <typename T> class
#ifdef VECTOR3_USE_PADDING
  __attribute__ ((aligned (4 * boost::alignment_of<T>::value)))
#endif
  Vector3 {
#ifdef VECTOR3_USE_ARRAY
    T data_[3];
#else
    T x_, y_, z_;
#endif
    //T _padding;

    class PrivateType {
      friend class Vector3;
      NVCC_HOST_DEVICE PrivateType () {}
    };

  public:
    NVCC_HOST_DEVICE Vector3 () {}
#if !defined (__CUDACC__)
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#ifdef VECTOR3_USE_ARRAY
    //NVCC_HOST_DEVICE explicit Vector3 (T v) { data_[0] = v; data_[1] = v; data_[2] = v; }
    NVCC_HOST_DEVICE Vector3 (T x, T y, T z) { data_[0] = x; data_[1] = y; data_[2] = z; }
    template <typename U> NVCC_HOST_DEVICE Vector3 (Vector3<U> v, UNUSED typename boost::enable_if<boost::is_convertible<U, T>, PrivateType>::type dummy = PrivateType ()) { data_[0] = v.x (); data_[1] = v.y (); data_[2] = v.z (); }
    template <typename U> NVCC_HOST_DEVICE explicit Vector3 (Vector3<U> v, UNUSED typename boost::disable_if<boost::is_convertible<U, T>, PrivateType>::type dummy = PrivateType ()) { data_[0] = (T) v.x (); data_[1] = (T) v.y (); data_[2] = (T) v.z (); }
#else
    //NVCC_HOST_DEVICE explicit Vector3 (T v) : x_ (v), y_ (v), z_ (v) {}
    NVCC_HOST_DEVICE Vector3 (T x, T y, T z) : x_ (x), y_ (y), z_ (z) {}
    template <typename U> NVCC_HOST_DEVICE Vector3 (Vector3<U> v, UNUSED typename boost::enable_if<boost::is_convertible<U, T>, PrivateType>::type dummy = PrivateType ()) : x_ (v.x ()), y_ (v.y ()), z_ (v.z ()) {}
    template <typename U> NVCC_HOST_DEVICE explicit Vector3 (Vector3<U> v, UNUSED typename boost::disable_if<boost::is_convertible<U, T>, PrivateType>::type dummy = PrivateType ()) : x_ (v.x ()), y_ (v.y ()), z_ (v.z ()) {}
#endif
#if !defined (__CUDACC__)
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic pop
#endif
#endif

    NVCC_HOST_DEVICE const T& x () const {
#ifdef VECTOR3_USE_ARRAY
      return data_[0];
#else
      return x_;
#endif
    }

    NVCC_HOST_DEVICE const T& y () const {
#ifdef VECTOR3_USE_ARRAY
      return data_[1];
#else
      return y_;
#endif
    }

    NVCC_HOST_DEVICE const T& z () const {
#ifdef VECTOR3_USE_ARRAY
      return data_[2];
#else
      return z_;
#endif
    }

    NVCC_HOST_DEVICE T& x () {
#ifdef VECTOR3_USE_ARRAY
      return data_[0];
#else
      return x_;
#endif
    }

    NVCC_HOST_DEVICE T& y () {
#ifdef VECTOR3_USE_ARRAY
      return data_[1];
#else
      return y_;
#endif
    }

    NVCC_HOST_DEVICE T& z () {
#ifdef VECTOR3_USE_ARRAY
      return data_[2];
#else
      return z_;
#endif
    }

    NVCC_HOST_DEVICE const T& operator[] (size_t i) const {
#ifdef VECTOR3_USE_ARRAY
      ASSERT (i >= 0 && i < 3);
      return data_[i];
#else
      if (i == 0)
        return x_;
      else if (i == 1)
        return y_;
      else if (i == 2)
        return z_;
      else
        ABORT ();
#endif
    }

    NVCC_HOST_DEVICE T& operator[] (size_t i) {
#ifdef VECTOR3_USE_ARRAY
      ASSERT (i >= 0 && i < 3);
      return data_[i];
#else
      if (i == 0)
        return x_;
      else if (i == 1)
        return y_;
      else if (i == 2)
        return z_;
      else
        ABORT ();
#endif
    }
  };

  // Operations on Vector3

#define RTS(op) __decltype ((*(T*)NULL) op (*(U*)NULL))
#define RT(op) Vector3<RTS(op)>
#ifdef __CUDACC__ // Workaround BugPlayground/nvcc-5.0-templates-1.cu
  template <typename T> static ERROR_ATTRIBUTE ("should not be called") Vector3<T> helperGetVector3Type (T t);
#define RT2(op) __decltype (helperGetVector3Type ((*(T*)NULL) op (*(U*)NULL)))
#define RETURN_RT(op) typedef RT2(op) Ty; return Ty
#else
#define RETURN_RT(op) return RT(op)
#endif
  template <typename T, typename U> NVCC_HOST_DEVICE inline RT(+) operator+ (Vector3<T> v1, Vector3<U> v2) {
    RETURN_RT(+) (v1.x () + v2.x (), v1.y () + v2.y (), v1.z () + v2.z ());
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline Vector3<T>& operator+= (Vector3<T>& v1, Vector3<U> v2) {
    v1.x () += v2.x (); v1.y () += v2.y (); v1.z () += v2.z ();
    return v1;
  }


  template <typename T, typename U> NVCC_HOST_DEVICE inline RT(-) operator- (Vector3<T> v1, Vector3<U> v2) {
    RETURN_RT(-) (v1.x () - v2.x (), v1.y () - v2.y (), v1.z () - v2.z ());
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline Vector3<T>& operator-= (Vector3<T>& v1, Vector3<U> v2) {
    v1.x () -= v2.x (); v1.y () -= v2.y (); v1.z () -= v2.z ();
    return v1;
  }


  template <typename T, typename U> NVCC_HOST_DEVICE inline RT(*) operator* (Vector3<T> v, U scalar) {
    RETURN_RT(*) (v.x () * scalar, v.y () * scalar, v.z () * scalar);
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline RT(*) operator* (T scalar, Vector3<U> v) {
    RETURN_RT(*) (scalar * v.x (), scalar * v.y (), scalar * v.z ());
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline Vector3<T>& operator*= (Vector3<T>& v, U scalar) {
    v.x () *= scalar; v.y () *= scalar; v.z () *= scalar;
    return v;
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline RT(/) operator/ (Vector3<T> v, U scalar) {
    RETURN_RT(/) (v.x () / scalar, v.y () / scalar, v.z () / scalar);
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline Vector3<T>& operator/= (Vector3<T>& v, U scalar) {
    v.x () /= scalar; v.y () /= scalar; v.z () /= scalar;
    return v;
  }

  template <typename T> NVCC_HOST_DEVICE inline bool operator== (Vector3<T> v1, Vector3<T> v2) {
    return v1.x () == v2.x () && v1.y () == v2.y () && v1.z () == v2.z ();
  }
  template <typename T> NVCC_HOST_DEVICE inline bool operator!= (Vector3<T> v1, Vector3<T> v2) {
    return !(v1 == v2);
  }

  template <typename T> struct Abs2Impl<Vector3<T> > {
    static __decltype(Math::abs2 (*(T*)0)) apply (Vector3<T> v) {
      return Math::abs2 (v.x ()) + Math::abs2 (v.y ()) + Math::abs2 (v.z ());
    }
  };

  // dot product
  template <typename T, typename U> NVCC_HOST_DEVICE inline RTS(*) operator* (Vector3<T> v1, Vector3<U> v2) {
    return v1.x () * v2.x () + v1.y () * v2.y () + v1.z () * v2.z ();
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline RT(*) crossProduct (Vector3<T> v1, Vector3<U> v2) {
    RETURN_RT(*) (v1.y () * v2.z () - v1.z () * v2.y (),
                  v1.z () * v2.x () - v1.x () * v2.z (),
                  v1.x () * v2.y () - v1.y () * v2.x ());
  }

  template <typename T, typename U> inline RTS(*) crossProductAbs (Vector3<T> v1, Vector3<U> v2) {
    return Math::abs (crossProduct (v1, v2));
  }

#undef RTS
#undef RT
#ifdef __CUDACC__ // Workaround BugPlayground/nvcc-5.0-templates-1.cu
#undef RT2
#undef RETURN_RT
#endif

  // Unary +/-
  template <typename T> NVCC_HOST_DEVICE inline Vector3<T> operator+ (Vector3<T> v) {
    return Vector3<T> (+v.x (), +v.y (), +v.z ());
  }
  template <typename T> NVCC_HOST_DEVICE inline Vector3<T> operator- (Vector3<T> v) {
    return Vector3<T> (-v.x (), -v.y (), -v.z ());
  }

  template <typename F> Vector3<F> inline real (Vector3<std::complex<F> > v) {
    return Vector3<F> (real (v.x ()), real (v.y ()), real (v.z ()));
  }

  template <typename F> Vector3<F> inline imag (Vector3<std::complex<F> > v) {
    return Vector3<F> (imag (v.x ()), imag (v.y ()), imag (v.z ()));
  }

  template <typename F> Vector3<std::complex<F> > inline conj (Vector3<std::complex<F> > v) {
    return Vector3<std::complex<F> > (conj (v.x ()), conj (v.y ()), conj (v.z ()));
  }
}

#endif // !MATH_VECTOR3_HPP_INCLUDED
