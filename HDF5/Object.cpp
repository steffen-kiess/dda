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

#include "Object.hpp"

#include <HDF5/File.hpp>
#include <HDF5/DataType.hpp>
#include <HDF5/DataSpace.hpp>
#include <HDF5/Attribute.hpp>

namespace HDF5 {
  void Object::checkType () const {
    if (!isValid ())
      return;
    H5I_type_t type = getType ();
    switch (type) {
    case H5I_GROUP:
    case H5I_DATATYPE:
    case H5I_DATASET:
      break;

    default:
      ABORT_MSG ("Not a object");
    }
  }

  File Object::file () const {
    return File (Exception::check ("H5Iget_file_id", H5Iget_file_id (handle ())));
  }

  ObjectReference Object::reference () const {
    ObjectReference ref;
    Exception::check ("H5Rcreate", H5Rcreate (&ref.value (), handle (), ".", H5R_OBJECT, -1));
    return ref;
  }

  Attribute Object::createAttribute (const std::string& name, const DataType& type, const DataSpace& space, AttributeCreatePropList acpl, /*AttributeAccess*/PropList aapl) const {
    return Attribute (Exception::check ("H5Acreate2", H5Acreate2 (handle (), name.c_str (), type.handle (), space.handle (), acpl.handleOrDefault (), aapl.handleOrDefault ())));
  }

  bool Object::existsAttribute (const std::string& name) const {
    return Exception::check ("H5Aexists", H5Aexists (handle (), name.c_str ())) != 0;
  }

  Attribute Object::openAttribute (const std::string& name, /*AttributeAccess*/PropList aapl) const {
    return Attribute (Exception::check ("H5Aopen", H5Aopen (handle (), name.c_str (), aapl.handleOrDefault ())));
  }
}
