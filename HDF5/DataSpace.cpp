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

#include "DataSpace.hpp"

#include <HDF5/Group.hpp>

namespace HDF5 {
  void DataSpace::checkType () const {
    if (!isValid ())
      return;
    if (getType () != H5I_DATASPACE)
      ABORT_MSG ("Not a dataspace");
  }

  DataSpace DataSpace::create (H5S_class_t type) {
    return DataSpace (Exception::check ("H5Screate", H5Screate (type)));
  }

  DataSpace DataSpace::createSimpleRank (int rank, const hsize_t* current_dims, const hsize_t* maxdims) {
    return DataSpace (Exception::check ("H5Screate_simple", H5Screate_simple (rank, current_dims, maxdims)));
  }

  bool DataSpace::isSimple () const {
    return Exception::check ("H5Sis_simple", H5Sis_simple (handle ())) != 0;
  }
  H5S_class_t DataSpace::getSimpleExtentType () const {
    return Exception::check ("H5Sget_simple_extent_type", H5Sget_simple_extent_type (handle ()));
  }
  void DataSpace::getSimpleExtentDims (hsize_t* dims, hsize_t* maxDims) const {
    Exception::check ("H5Sget_simple_extent_dims", H5Sget_simple_extent_dims (handle (), dims, maxDims));
  }
  int DataSpace::getSimpleExtentNdims () const {
    return Exception::check ("H5Sget_simple_extent_ndims", H5Sget_simple_extent_ndims (handle ()));
  }
}
