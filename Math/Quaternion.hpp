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

#ifndef MATH_QUATERNION_HPP_INCLUDED
#define MATH_QUATERNION_HPP_INCLUDED

// A quaternion (see http://en.wikipedia.org/wiki/Quaternion)

#include <Core/Util.hpp>

#include <Math/Abs.hpp>

#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>

namespace Math {
  template <typename ftype> class Quaternion {
    class PrivateType {
      friend class Quaternion;
      PrivateType () {}
    };
    
#ifdef SWIG_VISIBILITY_WORKAROUND
  public:
#endif
    typedef Quaternion<ftype> Q;

  private:
    ftype a_, b_, c_, d_;

    public:
    Quaternion () {}
    Quaternion (ftype a, ftype b, ftype c, ftype d)
    : a_ (a), b_ (b), c_ (c), d_ (d)
    {
    }

#if !defined (__CUDACC__)
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic push
#endif
#endif
#pragma GCC diagnostic ignored "-Wconversion"
    template <typename U> Quaternion (Quaternion<U> q, UNUSED typename boost::enable_if<boost::is_convertible<U, ftype>, PrivateType>::type dummy = PrivateType ()) : a_ (q.a ()), b_ (q.b ()), c_ (q.c ()), d_ (q.d ()) {}
#ifndef SWIG
    template <typename U> explicit Quaternion (Quaternion<U> q, UNUSED typename boost::disable_if<boost::is_convertible<U, ftype>, PrivateType>::type dummy = PrivateType ()) : a_ (q.a ()), b_ (q.b ()), c_ (q.c ()), d_ (q.d ()) {}
#endif
#if !defined (__CUDACC__)
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic pop
#endif
#endif

    const ftype& a () const { return a_; }
    ftype& a () { return a_; }

    const ftype& b () const { return b_; }
    ftype& b () { return b_; }

    const ftype& c () const { return c_; }
    ftype& c () { return c_; }

    const ftype& d () const { return d_; }
    ftype& d () { return d_; }

    Q conjugate () const {
      return Q (a (), -b (), -c (), -d ());
    }

    Q operator+ (Q q) const {
      return Q (a () + q.a (), b () + q.b (), c () + q.c (), d () + q.d ());
    }
    Q& operator+= (Q q) {
      a () += q.a (); b () += q.b (); c () += q.c (); d () += q.d ();
      return *this;
    }

    Q operator- (Q q) const {
      return Q (a () - q.a (), b () - q.b (), c () - q.c (), d () - q.d ());
    }
    Q& operator-= (Q q) {
      a () -= q.a (); b () -= q.b (); c () -= q.c (); d () -= q.d ();
      return *this;
    }

    Q operator* (ftype f) const {
      return Q (a () * f, b () * f, c () * f, d () * f);
    }
    Q& operator*= (ftype f) {
      a () *= f; b () *= f; c () *= f; d () *= f;
      return *this;
    }

    Q operator* (Q q) const {
      return Q (a () * q.a () - b () * q.b () - c () * q.c () - d () * q.d (),
                a () * q.b () + b () * q.a () + c () * q.d () - d () * q.c (),
                a () * q.c () - b () * q.d () + c () * q.a () + d () * q.b (),
                a () * q.d () + b () * q.c () - c () * q.b () + d () * q.a ());
    }
    Q& operator*= (Q q) {
      *this = *this * q;
      return *this;
    }

    Q operator/ (ftype f) const {
      return Q (a () / f, b () / f, c () / f, d () / f);
    }
    Q& operator/= (ftype f) {
      a () /= f; b () /= f; c () /= f; d () /= f;
      return *this;
    }
  };

  template <typename ftype> inline Quaternion<ftype> operator* (ftype f, Quaternion<ftype> q) {
    return q * f;
  }

#ifndef SWIG
  template <typename T> struct Abs2Impl<Quaternion<T> > {
    static __decltype(Math::abs2 (*(T*)0)) apply (Quaternion<T> q) {
      return Math::abs2 (q.a ()) + Math::abs2 (q.b ()) + Math::abs2 (q.c ()) + Math::abs2 (q.d ());
    }
  };
#endif

}

#endif // !MATH_QUATERNION_HPP_INCLUDED
