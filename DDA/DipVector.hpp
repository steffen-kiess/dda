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

#ifndef DDA_DIPVECTOR_HPP_INCLUDED
#define DDA_DIPVECTOR_HPP_INCLUDED

// A vector which is distributed over multiple opencl devices
// The size of the part on each GPU is determined by the DDAParams class.
//
// Used for storing polarization vectors on the GPU.

#include <OpenCL/MultiGpuVector.hpp>

#include <DDA/DDAParams.hpp>

namespace DDA {
  template <typename F, typename T = std::complex<F> > class DipVector : public OpenCL::MultiGpuVector<T> {
    const DDAParams<F>& ddaParams_;

    static std::vector<size_t> vecSizes (const DDAParams<F>& ddaParams) {
      std::vector<size_t> ret (ddaParams.procs () ());
      for (size_t i = 0; i < ddaParams.procs (); i++) {
        ret[i] = ddaParams.localVecSize (i);
      }
      return ret;
    }

  public:
    DipVector (const OpenCL::StubPool& pool, const std::vector<cl::CommandQueue>& queues, const DDAParams<F>& ddaParams, OpenCL::VectorAccounting& accounting = OpenCL::VectorAccounting::getNull (), const std::string& name = "") :
      OpenCL::MultiGpuVector<T> (pool, queues, vecSizes (ddaParams), accounting, name),
      ddaParams_ (ddaParams)
    {
    }

    const DDAParams<F>& ddaParams () const {
      return ddaParams_;
    }

    boost::shared_ptr<std::vector<T> > read (const std::vector<cl::CommandQueue>& queues) const {
      ASSERT (queues.size () == ddaParams ().procs ());
      boost::shared_ptr<std::vector<T> > result = boost::make_shared<std::vector<T> > (ddaParams ().vecSize ());
      std::fill (result->begin (), result->end (), T ());
      for (size_t i = 0; i < ddaParams ().procs (); i++)
        for (int j = 0; j < 3; j++)
          (*this)[i].read (queues[i], result->data () + ddaParams ().localVec0 (i) + ddaParams ().vecStride () * j, ddaParams ().localVecStride (i) * j, ddaParams ().localNvCount (i));
      return result;
    }

    template <typename Allocator>
    void write (const std::vector<cl::CommandQueue>& queues, const std::vector<T, Allocator>& data) {
      ASSERT (queues.size () == ddaParams ().procs ());
      ASSERT (data.size () == ddaParams ().vecSize ());
      for (size_t i = 0; i < ddaParams ().procs (); i++)
        for (int j = 0; j < 3; j++)
          (*this)[i].write (queues[i], data.data () + ddaParams ().localVec0 (i) + ddaParams ().vecStride () * j, ddaParams ().localVecStride (i) * j, ddaParams ().localNvCount (i));
    }
  };

  template <typename F> inline void swap (DipVector<F>& v1, DipVector<F>& v2) {
    v1.swap (v2);
  }

  CALL_MACRO_FOR_OPENCL_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, DipVector)
}

#endif // !DDA_DIPVECTOR_HPP_INCLUDED
