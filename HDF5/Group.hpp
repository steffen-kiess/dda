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

#ifndef HDF5_GROUP_HPP_INCLUDED
#define HDF5_GROUP_HPP_INCLUDED

// A HDF5 group

#include <Core/Util.hpp>
#include <Core/Assert.hpp>

#include <hdf5.h>

#include <boost/filesystem/path.hpp>

#include <HDF5/Object.hpp>

namespace HDF5 {
  class Group : public Object {
    void checkType () const;

  public:
    Group () {
    }

    explicit Group (const IdComponent& value) : Object (value) {
      checkType ();
    }

    // This constructor takes ownership of the object refered to by value
    explicit Group (hid_t value) : Object (value) {
      checkType ();
    }

    static Group create (const File& file, GroupCreatePropList gcpl = GroupCreatePropList (), GroupAccessPropList gapl = GroupAccessPropList ());

    Object open (const std::string& name, LinkAccessPropList lapl = LinkAccessPropList ()) const;
    bool exists (const std::string& name, LinkAccessPropList lapl = LinkAccessPropList ()) const;

    static void copyObject (const Group& srcLoc, const std::string& srcName, const Group& dstLoc, const std::string& dstName, ObjectCreatePropList ocpypl = ObjectCreatePropList (), LinkCreatePropList lcpl = LinkCreatePropList ());

    ObjectReference getReferenceIfExists (const std::string& name, LinkAccessPropList lapl = LinkAccessPropList ()) const;

    void link (const std::string& name, const Object& obj, LinkCreatePropList lcpl = LinkCreatePropList (), LinkAccessPropList lapl = LinkAccessPropList ()) const;
    void linkIfNotNull (const std::string& name, const Object& obj, LinkCreatePropList lcpl = LinkCreatePropList (), LinkAccessPropList lapl = LinkAccessPropList ()) const;

    std::vector<std::string> list (H5_index_t indexType = H5_INDEX_NAME, H5_iter_order_t order = H5_ITER_INC) const;
  };
}

#endif // !HDF5_GROUP_HPP_INCLUDED
