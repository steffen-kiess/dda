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

#ifndef HDF5_ATTRIBUTE_HPP_INCLUDED
#define HDF5_ATTRIBUTE_HPP_INCLUDED

// A HDF5 attribute

#include <Core/Util.hpp>
#include <Core/Assert.hpp>

#include <hdf5.h>

#include <boost/filesystem/path.hpp>

#include <HDF5/IdComponent.hpp>

namespace HDF5 {
  class DataType;

  class Attribute : public IdComponent {
    void checkType () const;

  public:
    Attribute () {
    }

    explicit Attribute (const IdComponent& value) : IdComponent (value) {
      checkType ();
    }

    // This constructor takes ownership of the object refered to by value
    explicit Attribute (hid_t value) : IdComponent (value) {
      checkType ();
    }

    void read (void* buf, const HDF5::DataType& memType) const;
    void write (const void* buf, const HDF5::DataType& memType) const;
  };
}

#endif // !HDF5_ATTRIBUTE_HPP_INCLUDED
