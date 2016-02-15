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

#include "DataType.hpp"

#include <HDF5/Group.hpp>

namespace HDF5 {
  void DataType::checkType () const {
    if (!isValid ())
      return;
    if (getType () != H5I_DATATYPE)
      ABORT_MSG ("Not a datatype");
  }

  DataType DataType::create (H5T_class_t class_, size_t size) {
    return DataType (Exception::check ("H5Tcreate", H5Tcreate (class_, size)));
  }

  DataType DataType::copy () {
    return DataType (Exception::check ("H5Tcopy", H5Tcopy (handle ())));
  }

  H5T_class_t DataType::getClass () const {
    return Exception::check ("H5Tget_class", H5Tget_class (handle ()));
  }

  size_t DataType::getSize () const {
    size_t result = H5Tget_size (handle ());
    if (!result)
      Exception::error ("H5Tget_size");
    return result;
  }

  bool DataType::equals (const DataType& other) const {
    return Exception::check ("H5Tequal", H5Tequal (handle (), other.handle ())) != 0;
  }

  std::vector<uint8_t> DataType::encode () const {
    size_t size = 0;
    Exception::check ("H5Tencode", H5Tencode (handle (), NULL, &size));
    std::vector<uint8_t> data (size);
    size_t size1 = size;
    Exception::check ("H5Tencode", H5Tencode (handle (), data.data (), &size));
    ASSERT (size1 == size);
    return data;
  }

  H5T_sign_t DataType::getSign () const {
    return Exception::check ("H5Tget_sign", H5Tget_sign (handle ()));
  }
}
