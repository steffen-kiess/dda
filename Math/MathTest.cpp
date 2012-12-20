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

#include <Core/OStream.hpp>

//#include <Math/Vector3.hpp>
#include <Math/Vector3IOS.hpp>
#include <Math/Abs.hpp>

#include <Math/DiagMatrix3.hpp>

#include <Math/QuaternionIOS.hpp>

#include <complex>

namespace Math {
  template <> struct Abs2Impl<std::string> {
    static std::string apply (std::string s) {
      return s+"1";
    }
  };
}

int main () {
  Math::Vector3<int> i (5, 6, 7);

  Core::OStream::getStdout () << i << std::endl;
  std::complex<double> d (4, 5);

  Core::OStream::getStdout () << Math::abs2 (4.5) << std::endl;
  Core::OStream::getStdout () << Math::abs2 (-4) << std::endl;
  Core::OStream::getStdout () << Math::abs2 (d) << std::endl;
  Core::OStream::getStdout () << Math::abs2 (i) << std::endl;

  Core::OStream::getStdout () << Math::abs (4.5) << std::endl;
  Core::OStream::getStdout () << Math::abs (-4) << std::endl;
  Core::OStream::getStdout () << Math::abs (d) << std::endl;
  Core::OStream::getStdout () << Math::abs (i) << std::endl;
  //Core::OStream::getStdout () << Math::abs (std::string ()) << std::endl;

  Core::OStream::getStdout () << (i * i) << std::endl;
  Core::OStream::getStdout () << Math::crossProduct (i, i) << std::endl;
  //Core::OStream::getStdout () << Math::crossProductAbs (i, i) << std::endl;

  Core::OStream::getStdout () << Math::abs ((char) -5) << std::endl;

  return 0;
}
