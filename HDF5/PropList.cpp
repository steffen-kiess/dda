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

#include "PropList.hpp"

#include <HDF5/Group.hpp>

namespace HDF5 {
  void PropList::checkType () const {
    if (!isValid ())
      return;
    if (getType () != H5I_GENPROP_LST)
      ABORT_MSG ("Not a property list");
  }

  PropList PropList::create (hid_t cls_id) {
    return PropList (Exception::check ("H5Pcreate", H5Pcreate (cls_id)));
  }

  hid_t PropList::getClass () const {
    return Exception::check ("H5Pget_class", H5Pget_class (handle ()));
  }
  bool PropList::isA (hid_t pclass) const {
    return Exception::check ("H5Pisa_class", H5Pisa_class (handle (), pclass)) != 0;
  }
  PropList PropList::copy () const {
    return PropList (Exception::check ("H5Pcopy", H5Pcopy (handle ())));
  }
}
