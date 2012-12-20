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

#ifndef EMSIM_LENGTH_HPP_INCLUDED
#define EMSIM_LENGTH_HPP_INCLUDED

// Class representing a length in m

#include <Core/Assert.hpp>

#include <Math/Vector3.hpp>
#include <Math/Vector3IOS.hpp>
#include <Math/Float.hpp>

#include <EMSim/Forward.hpp>

#include <ostream>

#include <stdint.h>

#include <boost/shared_ptr.hpp>

namespace EMSim {
  class Length {
    ldouble value_;

  public:
    Length () : value_ (0) {}
    Length (ldouble value) : value_ (value) {}
    operator long double () const { return value_; }

    ldouble value () const { return value_; }

    static Length zero () {
      return Length (0);
    }

    static Length fromM (ldouble value) {
      return Length (value);
    }

    static Length fromMicroM (ldouble value) {
      return fromM (value * 1e-6);
    }

    template <typename T> T valueAs () const {
      return static_cast<T> (value ());
    }

#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Wfloat-equal"
    bool operator== (const EMSim::Length& length) {
      return value () == length.value ();
    }
#if defined (__clang__) || GCC_VERSION_IS_ATLEAST(4, 6)
#pragma GCC diagnostic pop
#endif
    bool operator!= (const EMSim::Length& length) {
      return !(*this == length);
    }
  };

  std::istream& operator>>(std::istream& stream, EMSim::Length& out);
  std::ostream& operator<<(std::ostream& stream, const EMSim::Length& value);
}

#endif // !EMSIM_LENGTH_HPP_INCLUDED
