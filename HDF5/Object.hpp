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

#ifndef HDF5_OBJECT_HPP_INCLUDED
#define HDF5_OBJECT_HPP_INCLUDED

// Base class for all HDF5 objects stored in a file (except attributes)

#include <Core/Util.hpp>
#include <Core/Assert.hpp>

#include <hdf5.h>

#include <boost/filesystem/path.hpp>

#include <HDF5/BaseTypes.hpp>
#include <HDF5/IdComponent.hpp>
#include <HDF5/PropLists.hpp> // Needed for default constructors

namespace HDF5 {
  class File;
  class Attribute;
  class DataType;
  class DataSpace;

  class Object : public IdComponent {
    void checkType () const;

  public:
    Object () {
    }

    explicit Object (const IdComponent& value) : IdComponent (value) {
      checkType ();
    }

    // This constructor takes ownership of the object refered to by value
    explicit Object (hid_t value) : IdComponent (value) {
      checkType ();
    }

    File file () const;

    ObjectReference reference () const;

    Attribute createAttribute (const std::string& name, const DataType& type, const DataSpace& space, AttributeCreatePropList acpl = AttributeCreatePropList (), /*AttributeAccess*/PropList aapl = /*AttributeAccess*/PropList ()) const;
    bool existsAttribute (const std::string& name) const;
    Attribute openAttribute (const std::string& name, /*AttributeAccess*/PropList aapl = /*AttributeAccess*/PropList ()) const;
  };
}

#endif // !HDF5_OBJECT_HPP_INCLUDED
