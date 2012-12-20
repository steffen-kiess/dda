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

#ifndef HDF5_EXCEPTION_HPP_INCLUDED
#define HDF5_EXCEPTION_HPP_INCLUDED

// Error handling for HDF5

#include <Core/Util.hpp>
#include <Core/Assert.hpp>
#include <Core/Exception.hpp>

#include <hdf5.h>

namespace HDF5 {
  class Exception : public Core::Exception {
    std::string methodName_;

  public:
    Exception (const std::string& methodName);
    virtual ~Exception () throw ();

    virtual std::string message () const;

    const std::string& methodName () const {
      return methodName_;
    }

    static NORETURN error (const char* methodName);

#define C(T)                                            \
    static T check (const char* methodName, T value) {  \
      if (value < 0)                                    \
        error (methodName);                             \
      return value;                                     \
    }
    C(int) C(long)
    // C(hid_t)
    C(H5I_type_t) C(H5S_class_t) C(H5T_class_t)
#undef C
    template <typename T>
    static T* check (const char* methodName, T* value) {
      if (!value)
        error (methodName);
      return value;
    }
  };
}

#endif // !HDF5_EXCEPTION_HPP_INCLUDED
