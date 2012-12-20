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

// Create the property list types / the predefined data type accessors

#include <Core/OStream.hpp>

#include <HDF5/Exception.hpp>

#include <map>

#include <boost/foreach.hpp>

struct IdName {
  hid_t id;
  const char* name;
};
#define IDNAME(x) {x, #x}

static IdName classes[] = {
  // grep 'define H5P_' /usr/include/H5Ppublic.h | grep H5P_CLS_ | sed 's/#define //;s/[ \t].*//;s/^/  IDNAME (/;s/$/),/' | grep -v H5P_ROOT
  IDNAME (H5P_OBJECT_CREATE),
  IDNAME (H5P_FILE_CREATE),
  IDNAME (H5P_FILE_ACCESS),
  IDNAME (H5P_DATASET_CREATE),
  IDNAME (H5P_DATASET_ACCESS),
  IDNAME (H5P_DATASET_XFER),
  IDNAME (H5P_FILE_MOUNT),
  IDNAME (H5P_GROUP_CREATE),
  IDNAME (H5P_GROUP_ACCESS),
  IDNAME (H5P_DATATYPE_CREATE),
  IDNAME (H5P_DATATYPE_ACCESS),
  IDNAME (H5P_STRING_CREATE),
  IDNAME (H5P_ATTRIBUTE_CREATE),
  IDNAME (H5P_OBJECT_COPY),
  IDNAME (H5P_LINK_CREATE),
  IDNAME (H5P_LINK_ACCESS),
};

#define IDNAME2(x) {H5T_##x, #x}
static IdName types[] = {
  // grep H5_DLLVAR /usr/include/H5Tpublic.h | sed 's/.* H5T_//;s/_g.*//;s/^/  IDNAME2 (/;s/$/),/'
  IDNAME2 (IEEE_F32BE),
  IDNAME2 (IEEE_F32LE),
  IDNAME2 (IEEE_F64BE),
  IDNAME2 (IEEE_F64LE),
  IDNAME2 (STD_I8BE),
  IDNAME2 (STD_I8LE),
  IDNAME2 (STD_I16BE),
  IDNAME2 (STD_I16LE),
  IDNAME2 (STD_I32BE),
  IDNAME2 (STD_I32LE),
  IDNAME2 (STD_I64BE),
  IDNAME2 (STD_I64LE),
  IDNAME2 (STD_U8BE),
  IDNAME2 (STD_U8LE),
  IDNAME2 (STD_U16BE),
  IDNAME2 (STD_U16LE),
  IDNAME2 (STD_U32BE),
  IDNAME2 (STD_U32LE),
  IDNAME2 (STD_U64BE),
  IDNAME2 (STD_U64LE),
  IDNAME2 (STD_B8BE),
  IDNAME2 (STD_B8LE),
  IDNAME2 (STD_B16BE),
  IDNAME2 (STD_B16LE),
  IDNAME2 (STD_B32BE),
  IDNAME2 (STD_B32LE),
  IDNAME2 (STD_B64BE),
  IDNAME2 (STD_B64LE),
  IDNAME2 (STD_REF_OBJ),
  IDNAME2 (STD_REF_DSETREG),
  IDNAME2 (UNIX_D32BE),
  IDNAME2 (UNIX_D32LE),
  IDNAME2 (UNIX_D64BE),
  IDNAME2 (UNIX_D64LE),
  IDNAME2 (C_S1),
  IDNAME2 (FORTRAN_S1),
  IDNAME2 (VAX_F32),
  IDNAME2 (VAX_F64),
  IDNAME2 (NATIVE_SCHAR),
  IDNAME2 (NATIVE_UCHAR),
  IDNAME2 (NATIVE_SHORT),
  IDNAME2 (NATIVE_USHORT),
  IDNAME2 (NATIVE_INT),
  IDNAME2 (NATIVE_UINT),
  IDNAME2 (NATIVE_LONG),
  IDNAME2 (NATIVE_ULONG),
  IDNAME2 (NATIVE_LLONG),
  IDNAME2 (NATIVE_ULLONG),
  IDNAME2 (NATIVE_FLOAT),
  IDNAME2 (NATIVE_DOUBLE),
  IDNAME2 (NATIVE_LDOUBLE),
  IDNAME2 (NATIVE_B8),
  IDNAME2 (NATIVE_B16),
  IDNAME2 (NATIVE_B32),
  IDNAME2 (NATIVE_B64),
  IDNAME2 (NATIVE_OPAQUE),
  IDNAME2 (NATIVE_HADDR),
  IDNAME2 (NATIVE_HSIZE),
  IDNAME2 (NATIVE_HSSIZE),
  IDNAME2 (NATIVE_HERR),
  IDNAME2 (NATIVE_HBOOL),
  IDNAME2 (NATIVE_INT8),
  IDNAME2 (NATIVE_UINT8),
  IDNAME2 (NATIVE_INT_LEAST8),
  IDNAME2 (NATIVE_UINT_LEAST8),
  IDNAME2 (NATIVE_INT_FAST8),
  IDNAME2 (NATIVE_UINT_FAST8),
  IDNAME2 (NATIVE_INT16),
  IDNAME2 (NATIVE_UINT16),
  IDNAME2 (NATIVE_INT_LEAST16),
  IDNAME2 (NATIVE_UINT_LEAST16),
  IDNAME2 (NATIVE_INT_FAST16),
  IDNAME2 (NATIVE_UINT_FAST16),
  IDNAME2 (NATIVE_INT32),
  IDNAME2 (NATIVE_UINT32),
  IDNAME2 (NATIVE_INT_LEAST32),
  IDNAME2 (NATIVE_UINT_LEAST32),
  IDNAME2 (NATIVE_INT_FAST32),
  IDNAME2 (NATIVE_UINT_FAST32),
  IDNAME2 (NATIVE_INT64),
  IDNAME2 (NATIVE_UINT64),
  IDNAME2 (NATIVE_INT_LEAST64),
  IDNAME2 (NATIVE_UINT_LEAST64),
  IDNAME2 (NATIVE_INT_FAST64),
  IDNAME2 (NATIVE_UINT_FAST64),
};

static std::string cppName (const std::string name) {
  std::string cppName;
  bool cap = true;
  for (size_t j = 0; j < name.length (); j++) {
    if (name[j] >= 'a' && name[j] <= 'z' && cap) {
      cppName += static_cast<char> (name[j] - 'a' + 'A');
      cap = false;
    } else if (name[j] == ' ') {
      cap = true;
    } else {
      cppName += name[j];
    }
  }
  if (cppName.substr (0, 7) == "Dataset")
    cppName[4] = 'S';
  if (cppName.substr (0, 8) == "Datatype")
    cppName[4] = 'T';
  if (cppName == "Root")
    cppName = "";
  cppName += "PropList";
  return cppName;
}

int main () {
  std::vector<IdName> classesSorted;
  std::map<std::string, char> classesSortedDone;
  bool done;
  do {
    done = true;
    for (size_t i = 0; i < sizeof (classes) / sizeof (*classes); i++) {
      hid_t cls = classes[i].id;
      hid_t parent = HDF5::Exception::check ("H5Pget_class_parent", H5Pget_class_parent (cls));
      std::string name = HDF5::Exception::check ("H5Pget_class_name", H5Pget_class_name (cls));
      std::string pname = HDF5::Exception::check ("H5Pget_class_name", H5Pget_class_name (parent));
      bool foundParent = (pname == "root") || classesSortedDone.count (pname);
      bool found = classesSortedDone.count (name);
      if (!found && foundParent) {
        done = false;
        classesSorted.push_back (classes[i]);
        classesSortedDone[name] = 0;
      }
    }
  } while (!done);
  ASSERT (classesSorted.size () == sizeof (classes) / sizeof (*classes));

  BOOST_FOREACH (IdName idName, classesSorted) {
    hid_t cls = idName.id;
    std::string name = HDF5::Exception::check ("H5Pget_class_name", H5Pget_class_name (cls));
    hid_t parent = HDF5::Exception::check ("H5Pget_class_parent", H5Pget_class_parent (cls));
    std::string pname = HDF5::Exception::check ("H5Pget_class_name", H5Pget_class_name (parent));
    Core::OStream::getStdout () << "  PLIST (" << cppName (name) << ", " << cppName (pname) << ", " << idName.name << ")" << std::endl;
  }

  Core::OStream::getStdout () << std::endl;
  Core::OStream::getStdout () << std::endl;
  Core::OStream::getStdout () << std::endl;
  
  for (size_t i = 0; i < sizeof (types) / sizeof (*types); i++) {
    std::string cls;
    switch (HDF5::Exception::check ("H5Tget_class", H5Tget_class (types[i].id))) {
    case H5T_INTEGER: cls = "Integer"; break;
    case H5T_FLOAT: cls = "Float"; break;
    case H5T_BITFIELD: cls = "BitField"; break;
    case H5T_TIME: cls = "Time"; break;
    case H5T_STRING: cls = "String"; break;
    case H5T_REFERENCE: cls = "Reference"; break;
    case H5T_OPAQUE: cls = "Opaque"; break;
    default: ABORT ();
    }
    Core::OStream::getStdout () << "  DTYPE (" << types[i].name << ", " << cls << "Type)" << std::endl;
  }

  return 0;
}
