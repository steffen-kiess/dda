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

#include "DataSet.hpp"

#include <Core/CheckedCast.hpp>

#include <HDF5/DataType.hpp>
#include <HDF5/DataSpace.hpp>
#include <HDF5/File.hpp>

namespace HDF5 {
  void DataSet::checkType () const {
    if (!isValid ())
      return;
    if (getType () != H5I_DATASET)
      ABORT_MSG ("Not a dataset");
  }

  DataSet DataSet::create (const File& file, const DataType& type, const DataSpace& space, const DataSetCreatePropList& dcpl, const DataSetAccessPropList& dapl) {
    return DataSet (Exception::check ("H5Dcreate_anon", H5Dcreate_anon (file.handle (), type.handle (), space.handle (), dcpl.handleOrDefault (), dapl.handleOrDefault ())));
  }

  void DataSet::read (void* buf, const HDF5::DataType& memType, const HDF5::DataSpace& memSpace, const HDF5::DataSpace& fileSpace, DataTransferPropList xfpl) const {
    Exception::check ("H5Dread", H5Dread (handle (), memType.handle (), memSpace.handleOrAll (), fileSpace.handleOrAll (), xfpl.handleOrDefault (), buf));
  }
  
  void DataSet::write (const void* buf, const HDF5::DataType& memType, const HDF5::DataSpace& memSpace, const HDF5::DataSpace& fileSpace, DataTransferPropList xfpl) const {
    Exception::check ("H5Dwrite", H5Dwrite (handle (), memType.handle (), memSpace.handleOrAll (), fileSpace.handleOrAll (), xfpl.handleOrDefault (), buf));
  }
  
  DataSpace DataSet::getSpace () const {
    return DataSpace (Exception::check ("H5Dget_space", H5Dget_space (handle ())));
  }
  DataType DataSet::getDataType () const {
    return DataType (Exception::check ("H5Dget_type", H5Dget_type (handle ())));
  }

  void DataSet::vlenReclaim (void* buf, const DataType& type, const DataSpace& space, DataTransferPropList xfpl) {
    Exception::check ("H5Dvlen_reclaim", H5Dvlen_reclaim (type.handle (), space.handle (), xfpl.handleOrDefault (), buf));
  }

  DataSetAccessPropList DataSet::accessPropList () const {
    return DataSetAccessPropList (Exception::check ("H5Dget_access_plist", H5Dget_access_plist (handle ())));
  }
  DataSetCreatePropList DataSet::createPropList () const {
    return DataSetCreatePropList (Exception::check ("H5Dget_create_plist", H5Dget_create_plist (handle ())));
  }

  uint64_t DataSet::getOffset () const {
    haddr_t offset = H5Dget_offset (handle ());
    if (offset == HADDR_UNDEF)
      Exception::error ("H5Dget_offset");
    return Core::checked_cast<uint64_t> (offset);
  }
}
