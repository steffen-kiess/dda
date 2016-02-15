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

#include "CompoundType.hpp"

#include <Core/Memory.hpp>

#include <HDF5/Group.hpp>

#include <limits>

namespace HDF5 {
  void CompoundType::checkType () const {
    if (!isValid ())
      return;
    if (getClass () != H5T_COMPOUND)
      ABORT_MSG ("Not a compound datatype");
  }

  CompoundType CompoundType::create (size_t size) {
    return (CompoundType) DataType::create (H5T_COMPOUND, size);
  }

  void CompoundType::insert (const std::string& name, size_t offset, const DataType& field) const {
    Exception::check ("H5Tinsert", H5Tinsert (handle (), name.c_str (), offset, field.handle ()));
  }

  size_t CompoundType::nMembers () const {
    return Exception::check ("H5Tget_nmembers", H5Tget_nmembers (handle ()));
  }

  std::string CompoundType::memberName (size_t i) const {
    ASSERT (i <= std::numeric_limits<unsigned>::max ());
    unsigned i2 = static_cast<unsigned> (i);
    Core::MallocRefHolder<char> result (HDF5::Exception::check ("H5Tget_member_name", H5Tget_member_name (handle (), i2)));
    return result.p;
  }

  size_t CompoundType::memberOffset (size_t i) const {
    ASSERT (i <= std::numeric_limits<unsigned>::max ());
    unsigned i2 = static_cast<unsigned> (i);
    size_t result = H5Tget_member_offset (handle (), i2);
    if (result == 0) {
      // Check for errors, H5Tget_member_offset fails only if H5Tget_member_name also fails
      // http://www.hdfgroup.org/HDF5/doc/RM/RM_H5T.html#Datatype-GetMemberOffset
      memberName (i);
    }
    return result;
  }

  DataType CompoundType::memberType (size_t i) const {
    ASSERT (i <= std::numeric_limits<unsigned>::max ());
    unsigned i2 = static_cast<unsigned> (i);
    return DataType (Exception::check ("H5Tget_member_type", H5Tget_member_type (handle (), i2)));
  }
}
