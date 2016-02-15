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

#include "Attribute.hpp"

#include <HDF5/Group.hpp>
#include <HDF5/DataType.hpp>

namespace HDF5 {
  void Attribute::checkType () const {
    if (!isValid ())
      return;
    if (getType () != H5I_ATTR)
      ABORT_MSG ("Not an attribute");
  }

  void Attribute::read (void* buf, const HDF5::DataType& memType) const {
    Exception::check ("H5Aread", H5Aread (handle (), memType.handle (), buf));
  }
  void Attribute::write (const void* buf, const HDF5::DataType& memType) const {
    Exception::check ("H5Awrite", H5Awrite (handle (), memType.handle (), buf));
  }
}
