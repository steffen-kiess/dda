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

#include "File.hpp"

#include <HDF5/Group.hpp>

#include <boost/filesystem.hpp>

namespace HDF5 {
  void File::checkType () const {
    if (!isValid ())
      return;
    if (getType () != H5I_FILE)
      ABORT_MSG ("Not a file");
  }

  File File::open (const boost::filesystem::path& name, unsigned int flags, FileCreatePropList fcpl, FileAccessPropList fapl) {
    bool doCreate = false;
    if (flags & H5F_ACC_CREAT) { // create flag is set
      if ((flags & H5F_ACC_RDWR) == 0)
        ABORT_MSG ("Create is set but H5F_ACC_RDWR is not set");
      if ((flags & H5F_ACC_TRUNC) && (flags & H5F_ACC_EXCL))
        ABORT_MSG ("Both H5F_ACC_TRUNC and H5F_ACC_EXCL are set");
      if ((flags & H5F_ACC_TRUNC) == 0 && (flags & H5F_ACC_EXCL) == 0) { // neither Truncate nor Exclusive
        if (boost::filesystem::exists (name)) { // file already exists, use open
          doCreate = false;
          flags &= ~H5F_ACC_TRUNC;
        } else {
          doCreate = true;
        }
      } else {
        doCreate = true;
      }
    }
      
    if (doCreate) { // create
      flags &= ~(H5F_ACC_RDONLY | H5F_ACC_RDWR | H5F_ACC_CREAT);
      return File (Exception::check ("H5Fcreate", H5Fcreate (name.string ().c_str (), flags, fcpl.handleOrDefault (), fapl.handleOrDefault ())));
    } else { // open
      if ((flags & H5F_ACC_TRUNC) || (flags & H5F_ACC_EXCL)) // either Truncate or Exclusive
        ABORT_MSG ("Truncate or Exclusive set but Create not set");
      return File (Exception::check ("H5Fopen", H5Fopen (name.string ().c_str (), flags, fapl.handleOrDefault ())));
    }
  }
  bool File::isHDF5 (const boost::filesystem::path& name) {
    return Exception::check ("H5Fis_hdf5", H5Fis_hdf5 (name.string ().c_str ())) != 0;
  }

  Group File::rootGroup () const {
    return Group (Exception::check ("H5Gopen2", H5Gopen2 (handle (), ".", H5P_DEFAULT)));
  }

  void* File::getVFDHandle (FileAccessPropList fapl) const {
    void* result = NULL;
    Exception::check ("H5Fget_vfd_handle", H5Fget_vfd_handle (handle (), fapl.handleOrDefault (), &result));
    ASSERT (result != NULL);
    return result;
  }
  int File::getVFDHandleFD (FileAccessPropList fapl) const {
    return *(int*)getVFDHandle (fapl);
  }

  std::string File::getFileName () const {
    ssize_t size = Exception::check ("H5Fget_name", H5Fget_name (handle (), NULL, 0));
    std::vector<char> name (size + 1);
    ssize_t size2 = Exception::check ("H5Fget_name", H5Fget_name (handle (), name.data (), size + 1));
    ASSERT (size == size2);
    ASSERT (!name[size]);
    return std::string (name.data (), size);
  }
}
