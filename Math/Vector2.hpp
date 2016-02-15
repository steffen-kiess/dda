/*
 * Copyright (c) 2010-2013 Steffen Kieß
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

#ifndef MATH_VECTOR2_HPP_INCLUDED
#define MATH_VECTOR2_HPP_INCLUDED

#include <Core/Util.hpp>
#include <Core/Assert.hpp>

#include <Math/Forward.hpp>
#include <Math/Abs.hpp>

#include <complex>

#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/utility/enable_if.hpp>

//#define VECTOR2_USE_ARRAY

namespace Math {
  template <typename T> class
  Vector2 {
#ifdef VECTOR2_USE_ARRAY
    T data_[2];
#else
    T x_, y_;
#endif

    class PrivateType {
      friend class Vector2;
      NVCC_HOST_DEVICE PrivateType () {}
    };

  public:
    NVCC_HOST_DEVICE Vector2 () {}
#if !defined (__CUDACC__)
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Wconversion"
#endif
#ifdef VECTOR2_USE_ARRAY
    //NVCC_HOST_DEVICE explicit Vector2 (T v) { data_[0] = v; data_[1] = v }
    NVCC_HOST_DEVICE Vector2 (T x, T y) { data_[0] = x; data_[1] = y; }
    template <typename U> NVCC_HOST_DEVICE Vector2 (Vector2<U> v, UNUSED typename boost::enable_if<boost::is_convertible<U, T>, PrivateType>::type dummy = PrivateType ()) { data_[0] = v.x (); data_[1] = v.y (); }
    template <typename U> NVCC_HOST_DEVICE explicit Vector2 (Vector2<U> v, UNUSED typename boost::disable_if<boost::is_convertible<U, T>, PrivateType>::type dummy = PrivateType ()) { data_[0] = (T) v.x (); data_[1] = (T) v.y (); }
#else
    //NVCC_HOST_DEVICE explicit Vector2 (T v) : x_ (v), y_ (v) {}
    NVCC_HOST_DEVICE Vector2 (T x, T y) : x_ (x), y_ (y) {}
    template <typename U> NVCC_HOST_DEVICE Vector2 (Vector2<U> v, UNUSED typename boost::enable_if<boost::is_convertible<U, T>, PrivateType>::type dummy = PrivateType ()) : x_ (v.x ()), y_ (v.y ()) {}
    template <typename U> NVCC_HOST_DEVICE explicit Vector2 (Vector2<U> v, UNUSED typename boost::disable_if<boost::is_convertible<U, T>, PrivateType>::type dummy = PrivateType ()) : x_ (v.x ()), y_ (v.y ()) {}
#endif
#if !defined (__CUDACC__)
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic pop
#endif
#endif

    NVCC_HOST_DEVICE const T& x () const {
#ifdef VECTOR2_USE_ARRAY
      return data_[0];
#else
      return x_;
#endif
    }

    NVCC_HOST_DEVICE const T& y () const {
#ifdef VECTOR2_USE_ARRAY
      return data_[1];
#else
      return y_;
#endif
    }

    NVCC_HOST_DEVICE T& x () {
#ifdef VECTOR2_USE_ARRAY
      return data_[0];
#else
      return x_;
#endif
    }

    NVCC_HOST_DEVICE T& y () {
#ifdef VECTOR2_USE_ARRAY
      return data_[1];
#else
      return y_;
#endif
    }

    NVCC_HOST_DEVICE const T& operator[] (size_t i) const {
#ifdef VECTOR2_USE_ARRAY
      ASSERT (i >= 0 && i < 2);
      return data_[i];
#else
      if (i == 0)
        return x_;
      else if (i == 1)
        return y_;
      else
        ABORT ();
#endif
    }

    NVCC_HOST_DEVICE T& operator[] (size_t i) {
#ifdef VECTOR2_USE_ARRAY
      ASSERT (i >= 0 && i < 2);
      return data_[i];
#else
      if (i == 0)
        return x_;
      else if (i == 1)
        return y_;
      else
        ABORT ();
#endif
    }
  };

  // Operations on Vector2

#define RTS(op) __decltype ((*(T*)NULL) op (*(U*)NULL))
#define RT(op) Vector2<RTS(op)>
#ifdef __CUDACC__ // Workaround BugPlayground/nvcc-5.0-templates-1.cu
  template <typename T> static ERROR_ATTRIBUTE ("should not be called") Vector2<T> helperGetVector2Type (T t);
#define RT2(op) __decltype (helperGetVector2Type ((*(T*)NULL) op (*(U*)NULL)))
#define RETURN_RT(op) typedef RT2(op) Ty; return Ty
#else
#define RETURN_RT(op) return RT(op)
#endif
  template <typename T, typename U> NVCC_HOST_DEVICE inline RT(+) operator+ (Vector2<T> v1, Vector2<U> v2) {
    RETURN_RT(+) (v1.x () + v2.x (), v1.y () + v2.y ());
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline Vector2<T>& operator+= (Vector2<T>& v1, Vector2<U> v2) {
    v1.x () += v2.x (); v1.y () += v2.y ();
    return v1;
  }


  template <typename T, typename U> NVCC_HOST_DEVICE inline RT(-) operator- (Vector2<T> v1, Vector2<U> v2) {
    RETURN_RT(-) (v1.x () - v2.x (), v1.y () - v2.y ());
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline Vector2<T>& operator-= (Vector2<T>& v1, Vector2<U> v2) {
    v1.x () -= v2.x (); v1.y () -= v2.y ();
    return v1;
  }


  template <typename T, typename U> NVCC_HOST_DEVICE inline RT(*) operator* (Vector2<T> v, U scalar) {
    RETURN_RT(*) (v.x () * scalar, v.y () * scalar);
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline RT(*) operator* (T scalar, Vector2<U> v) {
    RETURN_RT(*) (scalar * v.x (), scalar * v.y ());
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline Vector2<T>& operator*= (Vector2<T>& v, U scalar) {
    v.x () *= scalar; v.y () *= scalar;
    return v;
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline RT(/) operator/ (Vector2<T> v, U scalar) {
    RETURN_RT(/) (v.x () / scalar, v.y () / scalar);
  }

  template <typename T, typename U> NVCC_HOST_DEVICE inline Vector2<T>& operator/= (Vector2<T>& v, U scalar) {
    v.x () /= scalar; v.y () /= scalar;
    return v;
  }

  template <typename T> NVCC_HOST_DEVICE inline bool operator== (Vector2<T> v1, Vector2<T> v2) {
    return v1.x () == v2.x () && v1.y () == v2.y ();
  }
  template <typename T> NVCC_HOST_DEVICE inline bool operator!= (Vector2<T> v1, Vector2<T> v2) {
    return !(v1 == v2);
  }

  template <typename T> struct Abs2Impl<Vector2<T> > {
    static __decltype(Math::abs2 (*(T*)0)) apply (Vector2<T> v) {
      return Math::abs2 (v.x ()) + Math::abs2 (v.y ());
    }
  };

  // dot product
  template <typename T, typename U> NVCC_HOST_DEVICE inline RTS(*) operator* (Vector2<T> v1, Vector2<U> v2) {
    return v1.x () * v2.x () + v1.y () * v2.y ();
  }

#undef RTS
#undef RT
#ifdef __CUDACC__ // Workaround BugPlayground/nvcc-5.0-templates-1.cu
#undef RT2
#undef RETURN_RT
#endif

  // Unary +/-
  template <typename T> NVCC_HOST_DEVICE inline Vector2<T> operator+ (Vector2<T> v) {
    return Vector2<T> (+v.x (), +v.y ());
  }
  template <typename T> NVCC_HOST_DEVICE inline Vector2<T> operator- (Vector2<T> v) {
    return Vector2<T> (-v.x (), -v.y ());
  }

  template <typename F> Vector2<F> inline real (Vector2<std::complex<F> > v) {
    return Vector2<F> (real (v.x ()), real (v.y ()));
  }

  template <typename F> Vector2<F> inline imag (Vector2<std::complex<F> > v) {
    return Vector2<F> (imag (v.x ()), imag (v.y ()));
  }

  template <typename F> Vector2<std::complex<F> > inline conj (Vector2<std::complex<F> > v) {
    return Vector2<std::complex<F> > (conj (v.x ()), conj (v.y ()));
  }
}

#endif // !MATH_VECTOR2_HPP_INCLUDED
