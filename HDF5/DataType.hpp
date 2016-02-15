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

#ifndef HDF5_DATATYPE_HPP_INCLUDED
#define HDF5_DATATYPE_HPP_INCLUDED

// A HDF5 data type
//
// Not that the copy constructor does *not* create a copy of the data type;
// it only creates a new reference pointing to the same data type object.
// You can use DataType::copy() to create a copy of the data type.

#include <Core/Util.hpp>
#include <Core/Assert.hpp>

#include <hdf5.h>

#include <boost/filesystem/path.hpp>

#include <HDF5/IdComponent.hpp>

namespace HDF5 {
  class DataType : public IdComponent {
    void checkType () const;

  public:
    DataType () {
    }

    explicit DataType (const IdComponent& value) : IdComponent (value) {
      checkType ();
    }

    // This constructor takes ownership of the object refered to by value
    explicit DataType (hid_t value) : IdComponent (value) {
      checkType ();
    }

    static DataType create (H5T_class_t class_, size_t size);

    DataType copy ();

    H5T_class_t getClass () const;

    size_t getSize () const;

    bool equals (const DataType& other) const;

    std::vector<uint8_t> encode () const;

    H5T_sign_t getSign () const;
  };
}

#endif // !HDF5_DATATYPE_HPP_INCLUDED
