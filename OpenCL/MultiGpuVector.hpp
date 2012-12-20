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

#ifndef OPENCL_MULTIGPUVECTOR_HPP_INCLUDED
#define OPENCL_MULTIGPUVECTOR_HPP_INCLUDED

// A vector which is distributed over multiple opencl devices

#include <OpenCL/Vector.hpp>

#include <boost/shared_ptr.hpp>

namespace OpenCL {
  template <typename F> class MultiGpuVector {
    MultiGpuVector& operator= (const MultiGpuVector& rhs);
    MultiGpuVector (const MultiGpuVector& o);

    std::vector<boost::shared_ptr<Vector<F> > > vectors;
    std::vector<cl::Device> devices;

  public:
    typedef F ElementType;

    MultiGpuVector (const StubPool& pool, const std::vector<cl::CommandQueue>& queues, std::vector<size_t> sizes, VectorAccounting& accounting = VectorAccounting::getNull (), const std::string& name = "") : vectors (sizes.size ()), devices (vectors.size ()) {
      ASSERT (queues.size () == vectorCount ());
      for (size_t i = 0; i < vectorCount (); i++) {
        devices[i] = queues[i].getInfo<CL_QUEUE_DEVICE> ();
        vectors[i].reset (new Vector<F> (pool, sizes[i], accounting, name));
        char c;
        if (vectors[i]->getSize ())
          queues[i].enqueueReadBuffer ((*this)[i].getData (), true, 0, 1, &c); // Touch buffer
      }    
    }

    MultiGpuVector (const StubPool& pool, const std::vector<cl::CommandQueue>& queues, csize_t size, VectorAccounting& accounting = VectorAccounting::getNull (), const std::string& name = "") : vectors (queues.size ()), devices (vectors.size ()) {
      for (size_t i = 0; i < vectorCount (); i++) {
        devices[i] = queues[i].getInfo<CL_QUEUE_DEVICE> ();
        vectors[i].reset (new Vector<F> (pool, size, accounting, name));
        char c;
        if (vectors[i]->getSize ())
          queues[i].enqueueReadBuffer ((*this)[i].getData (), true, 0, 1, &c); // Touch buffer
      }    
    }

    csize_t vectorCount () const {
      return vectors.size ();
    }

    Vector<F>& operator[] (csize_t i) {
      ASSERT (i < vectorCount ());
      return *vectors[i ()];
    }

    const Vector<F>& operator[] (csize_t i) const {
      ASSERT (i < vectorCount ());
      return *vectors[i ()];
    }

    const cl::Device& device (csize_t i) const {
      ASSERT (i < vectorCount ());
      return devices[i ()];
    }

    csize_t size (csize_t i) const {
      return (*this)[i].size ();
    }

    void writeCopies (const std::vector<cl::CommandQueue>& queues, const F* data) {
      for (size_t i = 0; i < vectorCount (); i++) {
        ASSERT (queues[i].getInfo<CL_QUEUE_DEVICE> () () == device (i) ());
        (*this)[i].write (queues[i], data);
      }
    }

    void writeCopies (const std::vector<cl::CommandQueue>& queues, const F* data, csize_t dstIndex, csize_t count) {
      for (size_t i = 0; i < vectorCount (); i++) {
        ASSERT (queues[i].getInfo<CL_QUEUE_DEVICE> () () == device (i) ());
        (*this)[i].write (queues[i], data, dstIndex, count);
      }
    }

    void writeCopies (const std::vector<cl::CommandQueue>& queues, const std::vector<F>& data) {
      for (size_t i = 0; i < vectorCount (); i++) {
        ASSERT (queues[i].getInfo<CL_QUEUE_DEVICE> () () == device (i) ());
        (*this)[i].write (queues[i], data);
      }
    }

    void swap (MultiGpuVector<F>& v2) {
      ASSERT (vectorCount () == v2.vectorCount ());
      for (size_t i = 0; i < vectorCount (); i++) {
        ASSERT (size (i) == v2.size (i));
      }
      using std::swap;
      swap (vectors, v2.vectors);
    }

    void setToZero (const std::vector<cl::CommandQueue>& queues) {
      for (size_t i = 0; i < vectorCount (); i++)
        (*this)[i].setToZero (queues[i]);
    }
  };

  template <typename F> inline void swap (MultiGpuVector<F>& v1, MultiGpuVector<F>& v2) {
    v1.swap (v2);
  }
}

#endif // !OPENCL_MULTIGPUVECTOR_HPP_INCLUDED
