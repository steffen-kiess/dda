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

#include "WindowsError.hpp"

#if OS_WIN

#include <Core/Assert.hpp>
#include <Core/Memory.hpp>

#include <sstream>

#include <windows.h>

namespace Core {
  WindowsError::~WindowsError () throw () {
  }

  std::string WindowsError::message () const {
    std::stringstream str;
    str << function () << ": " << errstr ();
    return str.str ();
  }

  WindowsError::ErrorNumType WindowsError::getLastError () {
    return GetLastError ();
  }

  std::string WindowsError::errnumToString (ErrorNumType errnum) {
    std::stringstream str;
    char* lpMsgBuf;
    if (!FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errnum, MAKELANGID (LANG_NEUTRAL, SUBLANG_NEUTRAL), (char*) &lpMsgBuf, 0, NULL)) {
      DWORD err2 = GetLastError ();
      str << "FormatMessage () for " << errnum << " returned " << err2;
      ABORT_MSG (str.str ());
    }
    Core::WindowsLocalRefHolder<char> refHolder (lpMsgBuf);
    size_t len = strlen (lpMsgBuf);
    if (len && lpMsgBuf[len - 1] == '\n') {
      lpMsgBuf[len - 1] = 0;
      if ((len > 1) && lpMsgBuf[len - 2] == '\r')
        lpMsgBuf[len - 2] = 0;
    }
    str << lpMsgBuf << " (" << errnum << ")";
    return str.str ();
  }
}

#endif // OS_WIN
