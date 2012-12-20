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

#ifndef DDA_TOSTRING_HPP_INCLUDED
#define DDA_TOSTRING_HPP_INCLUDED

// Methods for converting values to strings

#include <Core/Assert.hpp>
#include <Core/Exception.hpp>
#include <Core/Type.hpp>

#include <typeinfo>
#include <sstream>
#include <ostream>
#include <complex>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/any.hpp>

#include <Math/Vector3.hpp>
#include <Math/DiagMatrix3.hpp>

namespace DDA {
  template <typename T>
  static std::string toString (const T& t) {
    return boost::lexical_cast<std::string> (t);
  }

  template <typename T>
  static std::string toString (const std::complex<T>& t) {
    std::string imag = toString (t.imag ());
    if (imag.length () && imag[0] == '-')
      return toString (t.real ()) + imag + "i";
    else
      return toString (t.real ()) + "+" + imag + "i";
  }

  template <typename T>
  static std::string toString (const Math::Vector3<T>& t) {
    return "(" + toString (t.x ()) + ", " + toString (t.y ()) + ", " + toString (t.z ()) + ")";
  }

#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Wfloat-equal"
  template <typename T>
  static std::string toString (const Math::DiagMatrix3<T>& t) {
    if ((t.m11 () == t.m22 ())
        && (t.m11 () == t.m33 ()))
      return toString (t.m11 ());
    else
      return "(" + toString (t.m11 ()) + ", " + toString (t.m22 ()) + ", " + toString (t.m33 ()) + ")";
  }
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic pop
#endif

  std::vector<std::string> anyToStringList (const boost::any& p);
}

#endif // !DDA_TOSTRING_HPP_INCLUDED
