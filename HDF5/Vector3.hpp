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

#ifndef HDF5_VECTOR3_HPP_INCLUDED
#define HDF5_VECTOR3_HPP_INCLUDED

// Provides a HDF5 type for Math::Vector3<T>

#include <Core/Util.hpp>

#include <HDF5/Type.hpp>
#include <Math/Vector3.hpp>

namespace HDF5 {
  template <typename T> class TypeImpl<Math::Vector3<T> > {
#ifdef VECTOR3_USE_PADDING
#error "VECTOR3_USE_PADDING is defined"
#endif
    struct MyVector3 {
      T x, y, z;
    };
  
    HDF5_TYPE (MyVector3)
    HDF5_ADD_MEMBER (x)
    HDF5_ADD_MEMBER (y)
    HDF5_ADD_MEMBER (z)
    HDF5_TYPE_END
  };
}

#endif // !HDF5_VECTOR3_HPP_INCLUDED
