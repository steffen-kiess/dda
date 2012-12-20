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

#ifndef CORE_WINDOWSERROR_HPP_INCLUDED
#define CORE_WINDOWSERROR_HPP_INCLUDED

// A class "WindowsError" which can be thrown on windows errors
//
// Can be used like this: (will throw a WindowsError if QueryPerformanceCounter() fails)
// Core::Exception::check ("QueryPerformanceCounter", QueryPerformanceCounter (&time));

#include <Core/Util.hpp>

#if OS_WIN

#include <Core/Exception.hpp>

#include <stdint.h>

namespace Core {
  class WindowsError : public Exception {
  public:
    typedef uint32_t ErrorNumType; // uint32_t = DWORD

  private:
    std::string function_;
    ErrorNumType errnum_;

  public:
    WindowsError (std::string function, ErrorNumType errnum) : function_ (function), errnum_ (errnum) {
    }

    virtual ~WindowsError () throw ();

    virtual std::string message () const;

    const std::string& function () const {
      return function_;
    }

    ErrorNumType errnum () const {
      return errnum_;
    }

    static std::string errnumToString (ErrorNumType errnum);

    std::string errstr () const {
      return errnumToString (errnum ());
    }

    static ErrorNumType getLastError ();

    static inline NORETURN error (const std::string& function) {
      throw WindowsError (function, getLastError ());
    }
    static inline void errorIgnore (const char* function, ErrorNumType ignore) {
      ErrorNumType errnum = getLastError ();
        if (errnum != ignore)
          throw WindowsError (function, errnum);
    }

    static inline void check (const char* function) {
      ErrorNumType errnum = getLastError ();
      if (errnum != 0)
        throw WindowsError (function, errnum);
    }
    static inline void checkIgnore (const char* function, ErrorNumType ignore) {
      ErrorNumType errnum = getLastError ();
        if (errnum != 0 && errnum != ignore)
          throw WindowsError (function, errnum);
    }

    static inline uint32_t check (const char* function, uint32_t value) {
      if (value == 0)
        throw WindowsError (function, getLastError ());
      return value;
    }
    static inline uint32_t checkIgnore (const char* function, uint32_t value, ErrorNumType ignore) {
      if (value == 0) {
        ErrorNumType errnum = getLastError ();
        if (errnum != ignore)
          throw WindowsError (function, errnum);
      }
      return value;
    }

    template <typename T>
    static inline T* check (const char* function, T* value) {
      if (!value)
        throw WindowsError (function, getLastError ());
      return value;
    }
    template <typename T>
    static inline T* checkIgnore (const char* function, T* value, ErrorNumType ignore) {
      if (!value) {
        ErrorNumType errnum = getLastError ();
        if (errnum != ignore)
          throw WindowsError (function, errnum);
      }
      return value;
    }
  };
}

#endif // OS_WIN

#endif // !CORE_WINDOWSERROR_HPP_INCLUDED
