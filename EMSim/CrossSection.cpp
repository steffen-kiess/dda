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

#include "CrossSection.hpp"

#include <boost/lexical_cast.hpp>

namespace EMSim {
  template <typename T>
  static inline std::string str (const T& value) {
    //return boost::lexical_cast<std::string> (value);
    std::stringstream s;
    s.precision (10);
    s << value;
    return s.str ();
  }

  static inline std::string pad (std::string s, size_t length) {
    while (s.length () < length)
      s += ' ';
    return s;
  }

  void CrossSection::print (const Core::OStream& stream, const std::string& polLabel) const {
    stream << "Cext " << polLabel << " = " << pad (str (Cext * 1e12) + " um^2", 25) << " ";
    stream << "Qext " << polLabel << " = " << str (Qext) << std::endl;
    stream << "Cabs " << polLabel << " = " << pad (str (Cabs * 1e12) + " um^2", 25) << " ";
    stream << "Qabs " << polLabel << " = " << str (Qabs) << std::endl;
    stream << "Csca " << polLabel << " = " << pad (str (Csca * 1e12) + " um^2", 25) << " ";
    stream << "Qsca " << polLabel << " = " << str (Qsca) << std::endl;
  }
}
