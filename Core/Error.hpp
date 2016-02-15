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

#ifndef CORE_ERROR_HPP_INCLUDED
#define CORE_ERROR_HPP_INCLUDED

// A class "Error" which can be thrown on C runtime errors
//
// Can be used like this: (will throw an Error if symlink() fails)
// Core::Exception::check ("symlink", symlink ("a", "b"));

#include <Core/Exception.hpp>
#include <Core/Assert.hpp>
#include <Core/Util.hpp>

#include <boost/utility/enable_if.hpp>

#include <cerrno>

#include <ios>
#include <limits>

namespace Core {
  class Error : public Exception {
    std::string function_;
    int errnum_;

    class PrivateType {
      friend class Error;
      PrivateType () {}
    };

  public:
    Error (std::string function, int errnum) : function_ (function), errnum_ (errnum) {
    }

    virtual ~Error () throw ();

    virtual std::string message () const;

    const std::string& function () const {
      return function_;
    }

    int errnum () const {
      return errnum_;
    }

    static std::string errnumToString (int errnum);

    std::string errstr () const {
      return errnumToString (errnum ());
    }

    static inline NORETURN error (const char* function) {
      throw Error (function, errno);
    }
    static inline NORETURN error (const std::string& function) {
      throw Error (function, errno);
    }
    static inline void errorIgnore (const char* function, int ignore) {
      int errnum = errno;
        if (errnum != ignore)
          throw Error (function, errnum);
    }

    static inline void check (const char* function) {
      int errnum = errno;
      if (errnum != 0)
        throw Error (function, errnum);
    }
    static inline void checkIgnore (const char* function, int ignore) {
      int errnum = errno;
      if (errnum != 0 && errnum != ignore)
        throw Error (function, errnum);
    }

    template <typename T>
    static inline T check (const char* function, T value, UNUSED typename boost::enable_if_c<std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_signed, PrivateType>::type dummy = PrivateType ()) {
      if (value == -1)
        throw Error (function, errno);
      return value;
    }
    template <typename T>
    static inline T checkIgnore (const char* function, T value, int ignore, UNUSED typename boost::enable_if_c<std::numeric_limits<T>::is_integer && std::numeric_limits<T>::is_signed, PrivateType>::type dummy = PrivateType ()) {
      if (value == -1) {
        int errnum = errno;
        if (errnum != ignore)
          throw Error (function, errnum);
      }
      return value;
    }

    template <typename T>
    static inline T* check (const char* function, T* value) {
      if (!value)
        throw Error (function, errno);
      return value;
    }
    template <typename T>
    static inline T* checkIgnore (const char* function, T* value, int ignore) {
      if (!value) {
        int errnum = errno;
        if (errnum != ignore)
          throw Error (function, errnum);
      }
      return value;
    }

    class IosStreamFail : public Exception {
      std::string function_;

    public:
      IosStreamFail (std::string function) : function_ (function) {
      }

      virtual ~IosStreamFail () throw ();

      virtual std::string message () const;

      const std::string& function () const {
        return function_;
      }
    };
    // There is no guarantee that std::*stream sets errno, but it has no other
    // way to show it's error...
    template <typename T>
    static inline const std::basic_ios<T>& check (const char* function, const std::basic_ios<T>& value) {
      if (value.bad ()) {
        int errnum = errno;
        ASSERT_MSG (errnum != 0, "errno is 0");
        throw Error (function, errnum);
      }
      if (value.fail ()) {
        // Assume that errno doesn't get set when failbit ist set but not badbit
        throw IosStreamFail (function);
      }
      return value;
    }
    template <typename T>
    static inline const std::basic_ios<T>& checkIgnore (const char* function, const std::basic_ios<T>& value, int ignore) {
      if (value.bad ()) {
        int errnum = errno;
        if (errnum != ignore) {
          ASSERT_MSG (errnum != 0, "errno is 0");
          throw Error (function, errnum);
        }
      }
      if (value.fail ()) {
        // Assume that errno doesn't get set when failbit ist set but not badbit
        throw IosStreamFail (function);
      }
      return value;
    }
  };
}

#endif // !CORE_ERROR_HPP_INCLUDED
