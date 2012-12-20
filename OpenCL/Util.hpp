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

#ifndef OPENCL_UTIL_HPP_INCLUDED
#define OPENCL_UTIL_HPP_INCLUDED

// Some opencl utility functions

#include <Core/Util.hpp>

#include <OpenCL/Bindings.hpp>
#include <OpenCL/StubPool.hpp>
#include <OpenCL/Pointer.hpp>

namespace OpenCL {
  class Util {
    NO_COPY_CLASS (Util);

    boost::shared_ptr<class UtilStub> stub;
  public:
    explicit Util (const OpenCL::StubPool& pool);
    ~Util ();

    cl::Event memset (const cl::CommandQueue& queue, cl_mem s, csize_t offset, uint8_t c, csize_t n);
    cl::Event memset (const cl::CommandQueue& queue, const cl::Buffer& s, csize_t offset, uint8_t c, csize_t n);
    template <typename T, bool strong>
    cl::Event memset (const cl::CommandQueue& queue, const OpenCL::Pointer<T, strong>& pointer, uint8_t c, csize_t n) {
      return memset (queue, pointer.mem (), (csize_t) pointer.offset (), c, n);
    }

    uint32_t memoryToPointer (const cl::Buffer& mem);
  };
}

#endif // !OPENCL_UTIL_HPP_INCLUDED
