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

#ifndef MATH_VECTOR2IOS_HPP_INCLUDED
#define MATH_VECTOR2IOS_HPP_INCLUDED

// input and output operators for Math::Vector2
//
// Format is "(x, y)"

#include <Math/Vector2.hpp>

#include <Core/ParsingUtil.hpp>

#include <ostream>
#include <istream>

namespace Math {
  template <typename T> inline std::ostream& operator<< (std::ostream& stream, Vector2<T> vector) {
    stream << "(" << vector.x () << ", " << vector.y () << ")";
    return stream;
  }

  template <typename T> std::istream& operator>>(std::istream& stream, Vector2<T>& out) {
    if (!std::istream::sentry (stream))
      return stream;

    return stream >> Core::ExpectedChar ('(') >> Core::IgnoreWhitespace >> out.x () >> Core::ExpectedChar (',') >> Core::IgnoreWhitespace >> out.y () >> Core::ExpectedChar (')');
  }
}

#endif // !MATH_VECTOR2IOS_HPP_INCLUDED
