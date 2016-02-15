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

#ifndef HDF5_ARRAY_HPP_INCLUDED
#define HDF5_ARRAY_HPP_INCLUDED

// Implementation of matlab hdf5 serialization for Math::Array<>

#include <HDF5/Matlab.hpp>

#include <Math/Array.hpp>

namespace HDF5 {
  // TODO: Add h5MatlabLoad() for ArrayView<const T> and ArrayView<T>? (Could allocate an Array and return a view of the array)

  template <typename T, size_t N> struct MatlabSerializer<Math::ArrayView<const T, N> > {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const Math::ArrayView<const T, N>& array) {
      hsize_t dims[N];
      ptrdiff_t stride = sizeof (T);
      for (size_t i = 0; i < N; i++) {
        dims[N - 1 - i] = array.shape ()[i];
        ASSERT (stride == array.stridesBytes () [i]); // must have fortran order
        stride *= array.shape ()[i];
      }
      bool useNull = (stride == 0) && (H5_VERS_MAJOR < 1 || (H5_VERS_MAJOR == 1 && (H5_VERS_MINOR < 8 || (H5_VERS_MINOR == 8 && H5_VERS_RELEASE < 7))));
      HDF5::DataSpace dataSpace;
      if (useNull)
        dataSpace = HDF5::DataSpace::create (H5S_NULL);
      else
        dataSpace = HDF5::DataSpace::createSimpleRank (N, dims);
      HDF5::DataType memType = getMatlabH5MemoryType<T> ();
      HDF5::DataType fileType = getMatlabH5FileType<T> ();
      HDF5::DataSet dataSet = handle.createDataSet (fileType, dataSpace);
      writeAttribute (dataSet, "MATLAB_class", MatlabTypeImpl<T>::matlabClass ());
      if (!useNull)
        // pass in dataSpace as fileSpace to avoid problems when array.data () is NULL (causes "no output buffer" error)
        dataSet.write (array.data (), memType, dataSpace, dataSpace);
    }
  };

  template <typename T, size_t N> struct MatlabSerializer<Math::ArrayView<T, N> > {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const Math::ArrayView<T, N>& array) {
      MatlabSerializer<Math::ArrayView<const T, N> >::h5MatlabSave (handle, array);
    }
  };

  template <typename T, size_t N> struct MatlabSerializer<Math::Array<T, N> > {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const Math::Array<T, N>& array) {
      MatlabSerializer<Math::ArrayView<const T, N> >::h5MatlabSave (handle, array);
    }
    static inline void h5MatlabLoadDirect (const MatlabDeserializationContextHandleDirect<Math::Array<T, N> >& handle) {
      MatlabObject mo (handle.get ());
      boost::array<size_t, N> len;
      if (mo.isEmpty () && mo.isNullDataSpace ()) {
        for (size_t i = 0; i < N; i++)
          len[i] = 0;
      } else {
        //ASSERT (mo.size ().size () == N);
        ASSERT (mo.size ().size () == N || (mo.size ().size () <= N && mo.size ().size () >= 2));
        for (size_t i = 0; i < mo.size ().size () && i < N; i++)
          len[i] = mo.size ()[i];
        for (size_t i = N; i < mo.size ().size (); i++)
          ASSERT (mo.size ()[i] == 1);
        for (size_t i = mo.size ().size (); i < N; i++)
          len[i] = 1; // Assume missing dimensions have a size of 1
      }
      handle.ref ().recreate (len, true);
      HDF5::DataType dt = getMatlabH5MemoryType<T> ();
      if (!mo.isEmpty ())
        mo.dataSet ().read (handle.ref ().data (), dt, mo.dataSpace ());
    }
  };
}

#endif // !HDF5_ARRAY_HPP_INCLUDED
