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

#ifndef CORE_PARSINGUTIL_HPP_INCLUDED
#define CORE_PARSINGUTIL_HPP_INCLUDED

// Helper classes for parsing values

#include <Core/Util.hpp>

#include <istream>

namespace Core {
  namespace Intern {
    class IgnoreWhitespaceType {
    public:
      IgnoreWhitespaceType () {}
    };
    std::istream& operator>> (std::istream& stream, UNUSED IgnoreWhitespaceType ignore);
  }
  // Can be used like "istream >> Core::IgnoreWhitespace" to read all whitespace
  // from istream
  static const Intern::IgnoreWhitespaceType IgnoreWhitespace;

  class ExpectedChar {
    char expected_;
    bool ignoreWhitespace_;

  public:
    ExpectedChar (char expected, bool ignoreWhitespace = true) : expected_ (expected), ignoreWhitespace_ (ignoreWhitespace) {}

    char expected () const { return expected_; }
    bool ignoreWhitespace () const { return ignoreWhitespace_; }
  };
  // Can be used like "istream >> ExpectedChar ('X')" to read a character from
  // istream and set failbit if it is not 'X'
  std::istream& operator>> (std::istream& stream, ExpectedChar expected);
}

#endif // !CORE_PARSINGUTIL_HPP_INCLUDED
