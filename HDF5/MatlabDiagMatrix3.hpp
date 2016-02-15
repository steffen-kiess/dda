/*
 * Copyright (c) 2010-2013 Steffen Kieß
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

#ifndef HDF5_MATLABDIAGMATRIX3_HPP_INCLUDED
#define HDF5_MATLABDIAGMATRIX3_HPP_INCLUDED

#include <Math/DiagMatrix3.hpp>

#include <HDF5/Matlab.hpp>

namespace HDF5 {
  template <typename T> struct MatlabSerializer<Math::DiagMatrix3<T> > {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const Math::DiagMatrix3<T>& v) {
      HDF5::DataSpace dataSpace = HDF5::DataSpace::createSimple (3);
      HDF5::DataType memType = getMatlabH5MemoryType<T> ();
      HDF5::DataType fileType = getMatlabH5FileType<T> ();
      HDF5::DataSet dataSet = handle.createDataSet (fileType, dataSpace);
      writeAttribute (dataSet, "MATLAB_class", MatlabTypeImpl<T>::matlabClass ());
      dataSet.write (&v.diag ().x (), memType, dataSpace);
    }
    static inline void h5MatlabLoadDirect (const MatlabDeserializationContextHandleDirect<Math::DiagMatrix3<T> >& handle) {
      MatlabObject mo (handle.get ());
      mo.get1dValues (&handle.ref ().diag ().x (), 3);
    }
  };

  template <typename T> struct MatlabSerializer<std::vector<Math::DiagMatrix3<T> > > {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const std::vector<Math::DiagMatrix3<T> >& v) {
      bool useNull = (v.size () == 0) && (H5_VERS_MAJOR < 1 || (H5_VERS_MAJOR == 1 && (H5_VERS_MINOR < 8 || (H5_VERS_MINOR == 8 && H5_VERS_RELEASE < 7))));
      HDF5::DataSpace dataSpace;
      if (useNull)
        dataSpace = HDF5::DataSpace::create (H5S_NULL);
      else
        dataSpace = HDF5::DataSpace::createSimple (v.size (), 3);
      HDF5::DataType memType = getMatlabH5MemoryType<T> ();
      HDF5::DataType fileType = getMatlabH5FileType<T> ();
      HDF5::DataSet dataSet = handle.createDataSet (fileType, dataSpace);
      writeAttribute (dataSet, "MATLAB_class", MatlabTypeImpl<T>::matlabClass ());
      if (!useNull)
        // pass in dataSpace as fileSpace to avoid problems when v.data () is NULL (causes "no output buffer" error)
        dataSet.write (v.data (), memType, dataSpace, dataSpace);
    }
    static inline void h5MatlabLoadDirect (const MatlabDeserializationContextHandleDirect<std::vector<Math::DiagMatrix3<T> > >& handle) {
      MatlabObject mo (handle.get ());
      if (mo.isEmpty ()) {
        if (!mo.isNullDataSpace ()) {
          ASSERT (mo.size ().size () == 2);
          ASSERT (mo.size ()[0] == 0 || mo.size ()[0] == 3);
          ASSERT (mo.size ()[1] == 0);
        }
        handle.ref ().resize (0);
        return;
      }
      ASSERT (mo.size ().size () == 2);
      ASSERT (mo.size ()[0] == 3);
      size_t len = mo.size ()[1];

      handle.ref ().resize (len);
      HDF5::DataType type = getMatlabH5MemoryType<T> ();
      mo.dataSet ().read (handle.ref ().data (), type, mo.dataSpace ());
    }
  };
}

#endif // !HDF5_MATLABDIAGMATRIX3_HPP_INCLUDED
