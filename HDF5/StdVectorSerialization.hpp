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

#ifndef HDF5_STDVECTORSERIALIZATION_HPP_INCLUDED
#define HDF5_STDVECTORSERIALIZATION_HPP_INCLUDED

// Implementation of hdf5 serialization for std::vector<>

#include <Core/CheckedInteger.hpp>

#include <HDF5/Serialization.hpp>

#include <vector>

namespace HDF5 {
  template <typename T, typename Alloc> struct Serializer<std::vector<T, Alloc> > {
    static inline void h5Save (SerializationContext& context, const std::vector<T, Alloc>& vec) {
      HDF5::DataSpace space = HDF5::DataSpace::createSimple (vec.size ());
      HDF5::DataSet dataset = context.createDataSet (vec, getH5Type<T> (), space);
      dataset.write (vec.data (), getH5Type<T> ());
    }
    static inline void h5Load (DeserializationContext& context, ObjectReference name, HDF5::DataSet& dataSet) {
      HDF5::DataSpace space = dataSet.getSpace ();
      ASSERT (space.getSimpleExtentNdims () == 1);
      hsize_t size;
      space.getSimpleExtentDims (&size);
      boost::shared_ptr<std::vector<T, Alloc> > v (new std::vector<T, Alloc> (Core::checked_cast<size_t> (size)));
      context.registerValue (name, v);
      dataSet.read (v->data (), getH5Type<T> ());
    }
  };
}

#endif // !HDF5_STDVECTORSERIALIZATION_HPP_INCLUDED
