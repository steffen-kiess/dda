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

#include "TimeSpan.hpp"

#include <Core/Assert.hpp>

#include <sstream>

namespace Core {
  std::string TimeSpan::toString (bool appendUnit) const {
    std::stringstream str;
    
    str.setf (std::ios_base::internal, std::ios_base::adjustfield);
    str.setf (std::ios_base::fixed, std::ios_base::floatfield);
    str.setf (std::ios_base::showpoint);
    //str.width (10);
    str.precision (6);
    str << getSeconds ();
    if (appendUnit)
      str << "s";
    
    return str.str ();
  }

  TimeSpan TimeSpan::parse (const std::string& str, bool appendUnit) {
    std::string s = str;

    if (appendUnit) {
      ASSERT (str.length () > 0 && str[str.length () - 1] == 's');
      s = s.substr (0, str.length () - 1);
    }

    std::stringstream stream (s);
    double seconds;
    stream >> seconds;
    ASSERT (!stream.bad ());
    ASSERT (stream.eof ());

    return TimeSpan::fromSeconds (seconds);
  }
}
