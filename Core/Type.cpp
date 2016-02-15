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

#include "Type.hpp"

#include <Core/Assert.hpp>
#include <Core/Memory.hpp>

#include <cstdlib>

#ifndef _MSC_VER
#include <cxxabi.h>
#endif

namespace Core {
  namespace Type {
    std::string getName (const std::type_info& info) {
#ifndef _MSC_VER
      size_t len;
      int status;
      MallocRefHolder<char> demangled = abi::__cxa_demangle (info.name (), NULL, &len, &status);
      ASSERT_MSG (status == 0 || status == -2, std::string () + "abi::__cxa_demangle failed for `" + info.name () + "'");
      ASSERT_MSG (status == 0, std::string () + "abi::__cxa_demangle failed for `" + info.name () + "'");
      return demangled.p;
#else
      return info.name ();
#endif
    }
  }
}
