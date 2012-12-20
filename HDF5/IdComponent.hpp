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

#ifndef HDF5_IDCOMPONENT_HPP_INCLUDED
#define HDF5_IDCOMPONENT_HPP_INCLUDED

// Base class for all HDF5 objects
//
// Does automatic reference counting. Once the reference count drops to zero
// the HDF5 object is freed.

#include <Core/Util.hpp>
#include <Core/Assert.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <hdf5.h>

#include <HDF5/Exception.hpp>

namespace HDF5 {
  class IdComponent {
    class Shared {
      NO_COPY_CLASS (Shared);

    public:
      hid_t value;

      explicit Shared (hid_t value) : value (value) {
        ASSERT (value >= 0);
      }

      ~Shared () {
        Exception::check ("H5Idec_ref", H5Idec_ref (value));
      }
    };
    boost::shared_ptr<Shared> shared;

  public:
    IdComponent () {
    }

    // This constructor takes ownership of the object refered to by value
    explicit IdComponent (hid_t value) : shared (boost::make_shared<Shared> (value)) {
    }

    bool isValid () const {
      return shared.get () != NULL;
    }

    void assertValid () const {
      if (!isValid ())
        ABORT_MSG ("IdComponent is not initialized (is null)");
    }

    hid_t handle () const {
      assertValid ();
      return shared->value;
    }

    bool operator== (const IdComponent& other) const {
      if (!isValid ())
        return !other.isValid ();
      return handle () == other.handle ();
    }
    bool operator!= (const IdComponent& other) const {
      return !(*this == other);
    }

    H5I_type_t getType () const;
  };

  // Return an IdComponent without taking ownership of value
  inline IdComponent dontTakeOwnership (hid_t value) {
    Exception::check ("H5Iinc_ref", H5Iinc_ref (value));
    return IdComponent (value);
  }
}

#endif // !HDF5_IDCOMPONENT_HPP_INCLUDED
