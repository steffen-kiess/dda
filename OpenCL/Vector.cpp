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

#include "Vector.hpp"

#include "Vector.stub.hpp"

#include <Core/Assert.hpp>

#include <complex>

namespace OpenCL {
  namespace Intern {
    boost::shared_ptr<VectorStub> getVectorStub (const StubPool& pool) {
      return pool.get<VectorStub> ();
    }
  }

  namespace {
    class NullVectorAccounting : public VectorAccounting {
      std::auto_ptr<Handle> allocate (UNUSED const std::string& name, UNUSED const std::type_info& typeInfo, UNUSED csize_t count, UNUSED csize_t elementSize) {
        //Core::OStream::getStderr () << "Vector<" << Core::Type::getName (typeInfo) << "> Alloc " << ((count () ? (count * elementSize) : 1) + 1023) / 1024 << " kB" << " " <<  (0 ? Core::StackTrace (Core::StackTrace::createFromCurrentThread).toString () : "") << std::endl;
        return std::auto_ptr<Handle> ();
      }
    };
  }
  VectorAccounting& VectorAccounting::getNull () {
    static NullVectorAccounting instance;
    return instance;
  }


  template <typename F> void Vector<F>::setMultiply (const cl::CommandQueue& queue, const Vector& vector, F scalar) {
    ASSERT (getSize () == vector.getSize ());

    stub->multiplyScalar<F> (queue, getDefaultWorkItemCount (queue), mem, vector.mem, scalar, getUIntSize ());
  }

  template <typename F> void Vector<F>::setMultiplyElementwise (const cl::CommandQueue& queue, const Vector& vector1, const Vector& vector2) {
    ASSERT (getSize () == vector1.getSize ());
    ASSERT (getSize () == vector2.getSize ());

    stub->multiplyElementwise<F> (queue, getDefaultWorkItemCount (queue), mem, vector1.mem, vector2.mem, getUIntSize ());
  }

  template <typename F> void Vector<F>::setMultiplyElementwise (const cl::CommandQueue& queue, const Vector& vector1, const Vector& vector2, F scalar) {
    ASSERT (getSize () == vector1.getSize ());
    ASSERT (getSize () == vector2.getSize ());

    stub->multiplyElementwiseScalar<F> (queue, getDefaultWorkItemCount (queue), mem, vector1.mem, vector2.mem, scalar, getUIntSize ());
  }

  template class Vector<float>;
  template class Vector<std::complex<float> >;

  template class Vector<double>;
  template class Vector<std::complex<double> >;
}
