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

#ifndef HDF5_UTIL_HPP_INCLUDED
#define HDF5_UTIL_HPP_INCLUDED

// Some utility methods

#include <Core/Util.hpp>
#include <Core/Assert.hpp>
#include <HDF5/Type.hpp>
#include <HDF5/File.hpp>
#include <HDF5/Object.hpp>
#include <HDF5/DataSpace.hpp>
#include <HDF5/Attribute.hpp>

namespace HDF5 {
  template <typename T> inline void writeScalarAttribute (const HDF5::Object& obj, const std::string& name, const T& value) {
    obj.createAttribute (name, getH5Type<T> (), HDF5::DataSpace::create (H5S_SCALAR)).write (&value, getH5Type<T> ());
  }

  void writeAttribute (const HDF5::Object& obj, const std::string& name, const std::string& value);
}

#endif // !HDF5_UTIL_HPP_INCLUDED
