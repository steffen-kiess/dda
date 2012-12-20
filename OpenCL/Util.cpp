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

#include "Util.hpp"

#include <Core/Assert.hpp>

#include "Util.stub.hpp"

#include <Core/CheckedInteger.hpp>

namespace OpenCL {
  Util::Util (const OpenCL::StubPool& pool) {
    pool.set (stub);
  }

  Util::~Util () {
  }

  cl::Event Util::memset (const cl::CommandQueue& queue, cl_mem s, csize_t offset, uint8_t c, csize_t n) {
    if (c == 0 && offset % 4 == 0 && n % 4 == 0) {
      return stub->memsetZeroInt (queue, cl::NDRange (getDefaultWorkItemCount (queue)), cl::NDRange (), s, offset / 4, n / 4, OpenCL::getEvent);
    } else {
      return stub->memset (queue, cl::NDRange (getDefaultWorkItemCount (queue)), cl::NDRange (), s, offset, c, n, OpenCL::getEvent);
    }
  }

  cl::Event Util::memset (const cl::CommandQueue& queue, const cl::Buffer& s, csize_t offset, uint8_t c, csize_t n) {
    return memset (queue, s (), offset, c, n);
  }

  uint32_t Util::memoryToPointer (const cl::Buffer& mem) {
    cl::CommandQueue queue (stub->context (), stub->context ().getInfo<CL_CONTEXT_DEVICES> ()[0]);

    cl::Buffer target (stub->context (), CL_MEM_READ_WRITE, sizeof (uint32_t));

    stub->memoryToPointer (queue, target, mem);

    uint32_t value = -42;
    queue.enqueueReadBuffer (target, true, 0, sizeof (uint32_t), &value);
    return value;
  }
}
