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

#include <Core/Util.hpp>
#if OS_WIN
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#endif

#include "Error.hpp"

#include <Core/Assert.hpp>

#include <Core/StrError.h>

#include <sstream>
#include <vector>

#include <cstring>

namespace Core {
  Error::~Error () throw () {
  }

  std::string Error::message () const {
    std::stringstream str;
    str << function () << ": " << errstr ();
    return str.str ();
  }

  std::string Error::errnumToString (int errnum) {
    std::vector<char> str (8);

    int retval;
#if OS_UNIX
    do {
      errno = 0;
      // http://www.opengroup.org/onlinepubs/9699919799/functions/strerror.html
      retval = MY_XSI_strerror_r (errnum, str.data (), str.size ());

      // linux returns -1 and sets errno
      int err2 = errno;
      if (retval == -1 && err2 != 0)
        retval = err2;

      if (retval == ERANGE)
        str.resize (str.size () * 2);
    } while (retval == ERANGE);
#elif OS_WIN
    //return strerror (errnum); // Not thread safe
    for (;;) {
      str[str.size () - 1] = 0;
      retval = MY_strerror_s (str.data (), str.size () - 1, errnum);
      if (strlen (str.data ()) < str.size () - 2)
        break;
      str.resize (str.size () * 2);
    }
#else
#error
#endif

    if (retval == EINVAL) {
      std::stringstream s;
      s << "Unknown error number " << errnum;
      return s.str ();
    } else if (retval != 0) {
      std::stringstream s;
      s << "strerror_r for " << errnum << " returned " << retval;
      ABORT_MSG (s.str ());
    } else {
      return str.data ();
    }
  }

  Error::IosStreamFail::~IosStreamFail () throw () {
  }
  
  std::string Error::IosStreamFail::message () const {
    std::stringstream str;
    str << function () << ": Stream operation failed";
    return str.str ();
  }
}
