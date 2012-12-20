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

#ifndef DDA_GPUTRANSPOSEPLAN_HPP_INCLUDED
#define DDA_GPUTRANSPOSEPLAN_HPP_INCLUDED

// Implementation of transpose operations on the GPU

#include <Core/Profiling.hpp>
#include <Core/Util.hpp>

#include <OpenCL/Bindings.hpp>
#include <OpenCL/Vector.hpp>

#include <boost/scoped_ptr.hpp>
#include <boost/static_assert.hpp>

namespace DDA {
  struct GpuTransposeDimension {
    csize_t count;
    csize_t inputStride;
    csize_t outputStride;

    GpuTransposeDimension (csize_t count, csize_t inputStride, csize_t outputStride) : count (count), inputStride (inputStride), outputStride (outputStride) {}
  };

  class GpuTransposePlanStub; // Workaround http://llvm.org/bugs/show_bug.cgi?id=8505

  template <size_t elementSize> class GpuTransposePlanUntyped {
    boost::shared_ptr<class GpuTransposePlanStub> stub;
    OpenCL::Options options;
    //csize_t elementSize_;
    std::vector<GpuTransposeDimension> dimensions_;
    csize_t inputSize_;
    csize_t outputSize_;

  public:
    GpuTransposePlanUntyped (const OpenCL::StubPool& pool, /*csize_t elementSize, */const std::vector<GpuTransposeDimension>& dimensions, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
    ~GpuTransposePlanUntyped ();

    void transpose (const cl::CommandQueue& queue,
                    const cl::Buffer& input, csize_t inputOffset,
                    const cl::Buffer& output, csize_t outputOffset,
                    Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());

    csize_t inputSize () {
      return inputSize_;
    }

    csize_t outputSize () {
      return outputSize_;
    }
  };

  template <class T> class GpuTransposePlan {
    BOOST_STATIC_ASSERT ((sizeof (T) & (sizeof (T) - 1)) == 0);

    GpuTransposePlanUntyped<sizeof (T)> plan;

    template <class U> std::vector<U> v (U e1) {
      std::vector<U> r;
      r.push_back (e1);
      return r;
    }
    template <class U> std::vector<U> v (U e1, U e2) {
      std::vector<U> r;
      r.push_back (e1);
      r.push_back (e2);
      return r;
    }
    template <class U> std::vector<U> v (U e1, U e2, U e3) {
      std::vector<U> r;
      r.push_back (e1);
      r.push_back (e2);
      r.push_back (e3);
      return r;
    }
    template <class U> std::vector<U> v (U e1, U e2, U e3, U e4) {
      std::vector<U> r;
      r.push_back (e1);
      r.push_back (e2);
      r.push_back (e3);
      r.push_back (e4);
      return r;
    }

  public:
    GpuTransposePlan (const OpenCL::StubPool& pool, const std::vector<GpuTransposeDimension>& dimensions, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) :
      plan (pool, /*sizeof (T),*/ dimensions, prof) {
    }

    GpuTransposePlan (const OpenCL::StubPool& pool, GpuTransposeDimension d1) : plan (pool, /*sizeof (T),*/ v (d1)) {}
    GpuTransposePlan (const OpenCL::StubPool& pool, GpuTransposeDimension d1, GpuTransposeDimension d2) : plan (pool, /*sizeof (T),*/ v (d1, d2)) {}
    GpuTransposePlan (const OpenCL::StubPool& pool, GpuTransposeDimension d1, GpuTransposeDimension d2, GpuTransposeDimension d3) : plan (pool, /*sizeof (T),*/ v (d1, d2, d3)) {}
    GpuTransposePlan (const OpenCL::StubPool& pool, GpuTransposeDimension d1, GpuTransposeDimension d2, GpuTransposeDimension d3, GpuTransposeDimension d4) : plan (pool, /*sizeof (T),*/ v (d1, d2, d3, d4)) {}

    void transpose (const cl::CommandQueue& queue,
                    const OpenCL::Vector<T>& input, csize_t inputOffset,
                    OpenCL::Vector<T>& output, csize_t outputOffset,
                    Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) {
      ASSERT (inputOffset * sizeof (T) + plan.inputSize () <= input.getSize () * sizeof (T));
      ASSERT (outputOffset * sizeof (T) + plan.outputSize () <= output.getSize () * sizeof (T));
      ASSERT (&input != &output);

      plan.transpose (queue, input.getData (), inputOffset * sizeof (T), output.getDataWritable (), outputOffset * sizeof (T), prof);
    }
  };
}

#endif // !DDA_GPUTRANSPOSEPLAN_HPP_INCLUDED
