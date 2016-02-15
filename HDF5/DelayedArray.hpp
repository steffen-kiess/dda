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

#ifndef HDF5_DELAYEDARRAY_HPP_INCLUDED
#define HDF5_DELAYEDARRAY_HPP_INCLUDED

// HDF5::DelayedArray<> is a stub class for the hdf5 matlab serialization which
// when deserialized records the dimensions and a reference to the dataset and
// when serialized creates a data set with the given dimensions and stores
// a reference to the data set.
//
// Please note that serializing a deserialized object will not work (the created
// dataset will not be written to).

#include <Math/Array.hpp>

#include <HDF5/Matlab.hpp>

#include <boost/array.hpp>

namespace HDF5 {
  template <typename T, size_t N> class DelayedArray {
  public:
    DelayedArray () {
      for (size_t i = 0; i < N; i++)
        size[i] = 0;
      sizeUnknown = false;
    }

    boost::array<std::size_t, N> size;
    bool sizeUnknown;
    mutable HDF5::DataSet dataSet;

    void read (T* ptr) const {
      ASSERT (dataSet.isValid ());
      dataSet.read (ptr, getH5Type<T> ());
    }
    void write (const T* ptr) const {
      ASSERT (dataSet.isValid ());
      dataSet.write (ptr, getH5Type<T> ());
    }

    template <typename Assert>
    bool checkDimensionsStrides (const Math::ArrayView<const T, N, false, Math::ArrayConfig, Assert>& array) const {
      ptrdiff_t stride = sizeof (T);
      bool haveZero = false;
      for (size_t i = 0; i < N; i++) {
        size_t dsize = array.shape ()[i];
        if (!dsize)
          haveZero = true;
        if (!sizeUnknown)
          ASSERT (dsize == size[i]);
        ASSERT (stride == array.stridesBytes () [i]); // must have fortran order
        stride *= dsize;
      }
      if (sizeUnknown)
        ASSERT (haveZero);
      return !haveZero;
    }

    template <typename Assert>
    void read (const Math::ArrayView<T, N, false, Math::ArrayConfig, Assert>& array) const {
      if (checkDimensionsStrides (array))
        read (array.data ());
    }

    template <typename Assert>
    void write (const Math::ArrayView<const T, N, false, Math::ArrayConfig, Assert>& array) const {
      if (checkDimensionsStrides (array))
        write (array.data ());
    }
  };

  template <typename T, size_t N> struct MatlabSerializer<DelayedArray<T, N> > {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const DelayedArray<T, N>& array) {
      hsize_t dims[N];
      ptrdiff_t stride = 1;
      for (size_t i = 0; i < N; i++) {
        dims[N - 1 - i] = array.size[i];
        stride *= array.size[i];
      }
      bool useNull = (stride == 0) && (H5_VERS_MAJOR < 1 || (H5_VERS_MAJOR == 1 && (H5_VERS_MINOR < 8 || (H5_VERS_MINOR == 8 && H5_VERS_RELEASE < 7))));
      HDF5::DataSpace dataSpace;
      if (useNull)
        dataSpace = HDF5::DataSpace::create (H5S_NULL);
      else
        dataSpace = HDF5::DataSpace::createSimpleRank (N, dims);
      HDF5::DataType fileType = getMatlabH5FileType<T> ();
      HDF5::DataSet dataSet = handle.createDataSet (fileType, dataSpace);
      writeAttribute (dataSet, "MATLAB_class", MatlabTypeImpl<T>::matlabClass ());
      array.dataSet = dataSet;
    }
    static inline void h5MatlabLoadDirect (const MatlabDeserializationContextHandleDirect<DelayedArray<T, N> >& handle) {
      MatlabObject mo (handle.get ());

      if (mo.isNullDataSpace ()) {
        for (size_t i = 0; i < N; i++)
          handle.ref ().size[i] = 0;
        handle.ref ().sizeUnknown = true;
      } else {
        //ASSERT (mo.size ().size () == N);
        ASSERT (mo.size ().size () == N || (mo.size ().size () <= N && mo.size ().size () >= 2));
        for (size_t i = 0; i < mo.size ().size () && i < N; i++)
          handle.ref ().size[i] = mo.size ()[i];
        for (size_t i = N; i < mo.size ().size (); i++)
          ASSERT (mo.size ()[i] == 1);
        for (size_t i = mo.size ().size (); i < N; i++)
          handle.ref ().size[i] = 1; // Assume missing dimensions have a size of 1
      }

      handle.ref ().dataSet = mo.dataSet ();
    }
  };
}

#endif // !HDF5_DELAYEDARRAY_HPP_INCLUDED
