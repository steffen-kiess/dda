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

#include "StringUtil.hpp"

#include <Core/Assert.hpp>

#include <boost/algorithm/string.hpp>

namespace Core {
  std::string findReplace (const std::string& input, const std::string& find, const std::string& replace) {
    ASSERT (find.length () != 0);

    std::string s = input;
    for (size_t pos = 0; (pos = s.find (find, pos)) != std::string::npos;) {
      s.replace (pos, find.length (), replace);
      pos += replace.length ();
    }
    return s;
  }

  std::vector<std::string> split (const std::string& input, const std::string sep) {
    size_t pos = 0;
    size_t pos_old = 0;
    std::vector<std::string> result;

    while ((pos = input.find (sep, pos)) != std::string::npos) {
      result.push_back (input.substr (pos_old, pos - pos_old));
      pos = pos + 1;
      pos_old = pos;
    }
    result.push_back (input.substr (pos_old));

    return result;
  }

  std::string trim (const std::string& input) {
    return boost::trim_copy (input);
  }
}
