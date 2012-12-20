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

#ifndef HDF5_DATASPACE_HPP_INCLUDED
#define HDF5_DATASPACE_HPP_INCLUDED

// A HDF5 data space

#include <Core/Util.hpp>
#include <Core/Assert.hpp>

#include <hdf5.h>

#include <boost/filesystem/path.hpp>

#include <HDF5/IdComponent.hpp>

namespace HDF5 {
  class DataSpace : public IdComponent {
    void checkType () const;

  public:
    DataSpace () {
    }

    explicit DataSpace (const IdComponent& value) : IdComponent (value) {
      checkType ();
    }

    // This constructor takes ownership of the object refered to by value
    explicit DataSpace (hid_t value) : IdComponent (value) {
      checkType ();
    }

    static DataSpace create (H5S_class_t type);
    static DataSpace createSimpleRank (int rank, const hsize_t* current_dims, const hsize_t* maxdims = NULL);
    static DataSpace createSimple (hsize_t dim1) {
      hsize_t dims[] = {dim1};
      return createSimpleRank (1, dims);
    }
    static DataSpace createSimple (hsize_t dim1, hsize_t dim2 ) {
      hsize_t dims[] = {dim1, dim2};
      return createSimpleRank (2, dims);
    }
    static DataSpace createSimple (hsize_t dim1, hsize_t dim2, hsize_t dim3) {
      hsize_t dims[] = {dim1, dim2, dim3};
      return createSimpleRank (3, dims);
    }
    static DataSpace createSimple (hsize_t dim1, hsize_t dim2, hsize_t dim3, hsize_t dim4) {
      hsize_t dims[] = {dim1, dim2, dim3, dim4};
      return createSimpleRank (4, dims);
    }

    hid_t handleOrAll () const {
      if (!isValid ())
        return H5S_ALL;
      return handle ();
    }

    bool isSimple () const;
    H5S_class_t getSimpleExtentType () const;
    void getSimpleExtentDims (hsize_t* dims, hsize_t* maxDims = NULL) const;
    int getSimpleExtentNdims () const;
  };
}

#endif // !HDF5_DATASPACE_HPP_INCLUDED
