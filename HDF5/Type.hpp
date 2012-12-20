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

#ifndef HDF5_TYPE_HPP_INCLUDED
#define HDF5_TYPE_HPP_INCLUDED

// Provides HDF5::getH5Type<T>() which returns a HDF5 representation for T

#include <Core/Util.hpp>

#include <HDF5/BaseTypes.hpp>
#include <HDF5/DataType.hpp>
#include <HDF5/CompoundType.hpp>
#include <HDF5/DataTypes.hpp>
#include <HDF5/ComplexConversion.hpp>

#include <complex>

namespace HDF5 {
  template <typename T> struct TypeImpl {
    static HDF5::DataType createClassH5Type () {
      return T::createClassH5Type ();
    }
  };

  template <typename T> inline HDF5::DataType getH5Type () {
    static HDF5::DataType type = TypeImpl<T>::createClassH5Type ();
    return type;
  }
  template <typename T, typename U> inline HDF5::DataType getTargetH5Type (UNUSED T U::* ptr) {
    return getH5Type<T> ();
  }

#define HDF5_ADD_MEMBER_NAME(member, name)                              \
  ct.insert (name, offsetof (HDF5_CurrentType, member), ::HDF5::getTargetH5Type (&HDF5_CurrentType::member));
#define HDF5_ADD_MEMBER(member) HDF5_ADD_MEMBER_NAME (member, #member)
#define HDF5_TYPE(name)                                         \
  private:                                                      \
  typedef name HDF5_CurrentType;                                \
public:                                                         \
 static HDF5::DataType createClassH5Type () {                     \
 ::HDF5::CompoundType ct = ::HDF5::CompoundType::create (sizeof (HDF5_CurrentType));
#define HDF5_TYPE_END                           \
  return ct;                                    \
}

#define HDF5_SPECIALIZE_PREDTYPE(ty, pt)        \
  template <> class TypeImpl<ty> {              \
  public:                                       \
  static HDF5::DataType createClassH5Type () {  \
    ::HDF5::registerComplexConversion ();       \
    return HDF5::pt ();                         \
  }                                             \
  };
  HDF5_SPECIALIZE_PREDTYPE (float, NATIVE_FLOAT)
  HDF5_SPECIALIZE_PREDTYPE (double, NATIVE_DOUBLE)
  HDF5_SPECIALIZE_PREDTYPE (long double, NATIVE_LDOUBLE)
  HDF5_SPECIALIZE_PREDTYPE (uint8_t, NATIVE_UINT8)
  HDF5_SPECIALIZE_PREDTYPE (uint16_t, NATIVE_UINT16)
  HDF5_SPECIALIZE_PREDTYPE (uint32_t, NATIVE_UINT32)
  HDF5_SPECIALIZE_PREDTYPE (uint64_t, NATIVE_UINT64)
  HDF5_SPECIALIZE_PREDTYPE (int8_t, NATIVE_INT8)
  HDF5_SPECIALIZE_PREDTYPE (int16_t, NATIVE_INT16)
  HDF5_SPECIALIZE_PREDTYPE (int32_t, NATIVE_INT32)
  HDF5_SPECIALIZE_PREDTYPE (int64_t, NATIVE_INT64)
  HDF5_SPECIALIZE_PREDTYPE (ObjectReference, STD_REF_OBJ)
  template <> class TypeImpl<const char*> {
  public:
    static HDF5::DataType createClassH5Type () {
      HDF5::StringType ty = (HDF5::StringType) HDF5::C_S1 ().copy ();
      //ty.setSize (H5T_VARIABLE);
      Exception::check ("H5Tset_size", H5Tset_size (ty.handle (), H5T_VARIABLE));
      return ty;
    }
  };
  template <typename T> class TypeImpl<std::complex<T> > {
    struct MyComplex {
      T real, imag;
    };

    HDF5_TYPE (MyComplex)
    ::HDF5::registerComplexConversion ();
    HDF5_ADD_MEMBER (real)
    HDF5_ADD_MEMBER (imag)
    HDF5_TYPE_END
  };
}

#endif // !HDF5_TYPE_HPP_INCLUDED
