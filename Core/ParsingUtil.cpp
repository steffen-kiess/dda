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

#include "ParsingUtil.hpp"

namespace Core {
  namespace Intern {
    std::istream& operator>> (std::istream& stream, UNUSED IgnoreWhitespaceType ignore) {
      if (!std::istream::sentry (stream))
        return stream;

      for (;;) {
        int c = stream.peek ();
        if (!stream.good ())
          return stream;
        if (!(c == ' ' || c == '\t' || c == '\n' || c == '\r'))
          return stream;
        char c2;
        stream.get (c2);
      }
    }
  }

  std::istream& operator>> (std::istream& stream, ExpectedChar expected) {
    if (!std::istream::sentry (stream))
      return stream;

    if (expected.ignoreWhitespace ()) {
      stream >> IgnoreWhitespace;
      if (!stream.good ())
        return stream;
    }

    char c;
    stream.get (c);
    if (!stream.good ())
      return stream;

    if (c == expected.expected ())
      return stream;

    stream.putback (c);
    stream.setstate (std::ios::failbit);
    return stream;
  }
}
