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

#include "Length.hpp"

#include <sstream>

#include <cstdio>

namespace EMSim {
  std::istream& operator>>(std::istream& stream, Length& out) {
    if (!std::istream::sentry (stream))
      return stream;

    ldouble value;
    stream >> value;
  
    if (stream.fail ())
      return stream;
    stream.clear (stream.rdstate () & ~std::ios_base::eofbit);

    std::stringstream str;
    int c = stream.peek ();
    while (c != EOF && c != ',' && c != ')' && c != '-' && c != '+') {
      str << (char) stream.get ();
      c = stream.peek ();
    }
  
    std::string s = str.str ();
    ldouble factor;
    if (s == "" && value == 0) { // Special case: 0 does not need a unit
      out = Length (0);
      return stream;
    } else if (s == "µm" || s == "um") {
      factor = 1e-6l;
    } else if (s == "nm") {
      factor = 1e-9l;
    } else if (s == "mm") {
      factor = 1e-3l;
    } else if (s == "m") {
      factor = 1e+0l;
    } else {
      stream.setstate (std::ios::failbit);
      return stream;
    }
    out = Length (value * factor);
    return stream;
  }

  std::ostream& operator<<(std::ostream& stream, const Length& value) {
    if (!value.value ()) // Special case: 0 does not get a unit
      return stream << "0";
    //return stream << value.value () << "m";
    return stream << (value.value () * 1e+6) << "um";
  }
}
