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

#include "Group.hpp"

#include <HDF5/File.hpp>

namespace HDF5 {
  void Group::checkType () const {
    if (!isValid ())
      return;
    if (getType () != H5I_GROUP)
      ABORT_MSG ("Not a group");
  }

  Group Group::create (const File& file, GroupCreatePropList gcpl, GroupAccessPropList gapl) {
    return Group (Exception::check ("H5Gcreate_anon", H5Gcreate_anon (file.handle (), gcpl.handleOrDefault (), gapl.handleOrDefault ())));
  }

  Object Group::open (const std::string& name, LinkAccessPropList lapl) const {
    return Object (Exception::check ("H5Oopen", H5Oopen (handle (), name.c_str (), lapl.handleOrDefault ())));
  }
  bool Group::exists (const std::string& name, LinkAccessPropList lapl) const {
    return Exception::check ("H5Lexists", H5Lexists (handle (), name.c_str (), lapl.handleOrDefault ())) != 0;
  }

  void Group::copyObject (const Group& srcLoc, const std::string& srcName, const Group& dstLoc, const std::string& dstName, ObjectCreatePropList ocpypl, LinkCreatePropList lcpl) {
    Exception::check ("H5Ocopy", H5Ocopy (srcLoc.handle (), srcName.c_str (), dstLoc.handle (), dstName.c_str (), ocpypl.handleOrDefault (), lcpl.handleOrDefault ()));
  }

  ObjectReference Group::getReferenceIfExists (const std::string& name, LinkAccessPropList lapl) const {
    if (exists (name, lapl))
      return open (name, lapl).reference ();
    else
      return ObjectReference ();
  }

  void Group::link (const std::string& name, const Object& obj, LinkCreatePropList lcpl, LinkAccessPropList lapl) const {
    Exception::check ("H5Olink", H5Olink (obj.handle (), handle (), name.c_str (), lcpl.handleOrDefault (), lapl.handleOrDefault ()));
  }
  void Group::linkIfNotNull (const std::string& name, const Object& obj, LinkCreatePropList lcpl, LinkAccessPropList lapl) const {
    if (obj.isValid ())
      link (name, obj, lcpl, lapl);
  }

  namespace {
    herr_t listCallback (UNUSED hid_t group, const char* name, UNUSED const H5L_info_t* info, void* op_data) {
      std::vector<std::string>& names = *(std::vector<std::string>*) op_data;
      names.push_back (name);
      return 0;
    }
  }
  std::vector<std::string> Group::list (H5_index_t indexType, H5_iter_order_t order) const {
    std::vector<std::string> result;
    HDF5::Exception::check ("H5Literate", H5Literate (handle (), indexType, order, NULL, listCallback, &result));
    return result;
  }
}
