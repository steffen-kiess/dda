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

#ifndef HDF5_BASETYPES_HPP_INCLUDED
#define HDF5_BASETYPES_HPP_INCLUDED

// HDF5::ObjectReference represents an HDF5 object reference

#include <Core/Util.hpp>

#include <H5Rpublic.h>

namespace HDF5 {
  class File;
  class Object;

  class ObjectReference {
    hobj_ref_t value_;
  public:
    ObjectReference () : value_ (0) {}
    explicit ObjectReference (hobj_ref_t value) : value_ (value) {}
    hobj_ref_t value () const { return value_; }
    hobj_ref_t& value () { return value_; }
    
    bool isNull () const {
      return value () == 0;
    }

#define OP(op)                                                \
    bool operator op (const ObjectReference& other) const {   \
      return value () op other.value ();                      \
    }
    OP(==) OP(!=) OP(<) OP(>) OP(<=) OP(>=)
#undef OP

    Object dereference (const File& file) const;
  };
}

#endif // !HDF5_BASETYPES_HPP_INCLUDED
