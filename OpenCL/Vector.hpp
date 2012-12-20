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

#ifndef OPENCL_VECTOR_HPP_INCLUDED
#define OPENCL_VECTOR_HPP_INCLUDED

// A opencl vector class

#include <Core/Assert.hpp>
#include <Core/CheckedInteger.hpp>
#include <Core/Type.hpp>

#include <OpenCL/Bindings.hpp>
#include <OpenCL/Util.hpp>
#include <OpenCL/StubPool.hpp>
#include <OpenCL/Pointer.hpp>

#include <boost/scoped_ptr.hpp>

#include <memory>

namespace OpenCL {
  class VectorStub;

  namespace Intern {
    boost::shared_ptr<VectorStub> getVectorStub (const StubPool& pool);
  }

  template <typename F> class Vector;
  class VectorAccounting {
  public:
    class Handle {
    public:
      virtual ~Handle () {}
    };
    virtual std::auto_ptr<Handle> allocate (const std::string& name, const std::type_info& typeInfo, csize_t count, csize_t elementSize) = 0;

    static VectorAccounting& getNull ();
  };

  template <typename F> class Vector {
    Vector& operator= (const Vector& rhs);
    Vector (const Vector& o);

    boost::shared_ptr<class VectorStub> stub;
    size_t size_;
    cl::Buffer mem;
    std::auto_ptr<VectorAccounting::Handle> accountingHandle;
    Util util;

  public:
    typedef F ElementType;

    Vector (const StubPool& pool, csize_t size, VectorAccounting& accounting = VectorAccounting::getNull (), const std::string& name = "");

    void write (const cl::CommandQueue& queue, const F* data);
    void write (const cl::CommandQueue& queue, const F* data, csize_t dstIndex, csize_t count);
    void write (const cl::CommandQueue& queue, const std::vector<F>& data);

    void read (const cl::CommandQueue& queue, F* data) const;
    void read (const cl::CommandQueue& queue, F* data, csize_t dstIndex, csize_t count) const;
    void read (const cl::CommandQueue& queue, std::vector<F>& data) const;

    size_t getSize () const {
      return size_;
    }

    csize_t size () const {
      return size_;
    }

    uint32_t getUIntSize () const {
      return Core::checked_cast<uint32_t> (getSize ());
    }

    const cl::Buffer& getData () const {
      return mem;
    }

    const cl::Buffer& getDataWritable () {
      return mem;
    }

    void setMultiply (const cl::CommandQueue& queue, const Vector& vector, F scalar);
    void setMultiplyElementwise (const cl::CommandQueue& queue, const Vector& vector1, const Vector& vector2);
    void setMultiplyElementwise (const cl::CommandQueue& queue, const Vector& vector1, const Vector& vector2, F scalar);

    Pointer<const F> pointer () const {
      return Pointer<const F> (getData (), 0);
    }
    Pointer<const F> pointer (csize_t offset) const {
      ASSERT (offset <= size ()); 
      return Pointer<const F> (getData (), 0) + (cptrdiff_t) offset;
    }

    Pointer<F> pointer () {
      return Pointer<F> (getDataWritable (), 0);
    }
    Pointer<F> pointer (csize_t offset) {
      ASSERT (offset <= size ()); 
      return Pointer<F> (getDataWritable (), 0) + (cptrdiff_t) offset;
    }

    void swap (Vector<F>& v2) {
      ASSERT (size () == v2.size ());
      using std::swap;
      swap (mem, v2.mem);
    }

    void setToZero (const cl::CommandQueue& queue) {
      util.memset (queue, pointer (), 0, size () * sizeof (F));
    }
  };

  template <typename F> inline void swap (Vector<F>& v1, Vector<F>& v2) {
    v1.swap (v2);
  }
}

// IMPLEMENTATION
namespace OpenCL {
  template <typename F> Vector<F>::Vector (const StubPool& pool, csize_t size, VectorAccounting& accounting, const std::string& name) : stub (Intern::getVectorStub (pool)), accountingHandle (accounting.allocate (name, typeid (F), size (), sizeof (F))), util (pool) {
    this->size_ = size ();
    if (size_) {
      this->mem = cl::Buffer (pool.context (), 0, (size * sizeof(F)) ());
      if (0) {
        cl::CommandQueue queue (pool.context (), pool.context ().getInfo<CL_CONTEXT_DEVICES> ()[0]);
        Util (pool).memset (queue, mem, 0, 255, size_ * sizeof (F));
        queue.finish ();
      }
    } else {
      this->mem = cl::Buffer ();
    }
  }

  template <typename F> inline void Vector<F>::write (const cl::CommandQueue& queue, const F* data) {
    write (queue, data, 0, getSize ());
  }

  template <typename F> inline void Vector<F>::write (const cl::CommandQueue& queue, const F* data, csize_t dstIndex, csize_t count) {
    ASSERT (dstIndex + count <= getSize ());

    if (count != 0)
      queue.enqueueWriteBuffer (mem, true, (dstIndex * sizeof (F)) (), (count * sizeof (F)) (), (void*) data);
  }

  template <typename F> inline void Vector<F>::write (const cl::CommandQueue& queue, const std::vector<F>& data) {
    ASSERT (data.size () == size ());
    write (queue, data.data ());
  }

  template <typename F> inline void Vector<F>::read (const cl::CommandQueue& queue, F* data) const {
    read (queue, data, 0, getSize ());
  }

  template <typename F> inline void Vector<F>::read (const cl::CommandQueue& queue, F* data, csize_t dstIndex, csize_t count) const {
    ASSERT (dstIndex + count <= getSize ());

    if (count != 0)
      queue.enqueueReadBuffer (mem, true, (dstIndex * sizeof (F)) (), (count * sizeof (F)) (), data);
  }

  template <typename F> inline void Vector<F>::read (const cl::CommandQueue& queue, std::vector<F>& data) const {
    ASSERT (data.size () == size ());
    read (queue, data.data ());
  }
}

#endif // !OPENCL_VECTOR_HPP_INCLUDED
