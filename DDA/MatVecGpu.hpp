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

#ifndef DDA_MATVECGPU_HPP_INCLUDED
#define DDA_MATVECGPU_HPP_INCLUDED

// This implementation of MatVec will copy the input data to the GPU, use a
// GpuMatVec instance to do the matrix-vector-product and copy the results back
// to the CPU.

#include <Core/Profiling.hpp>

#include <DDA/MatVec.hpp>
#include <DDA/GpuMatVec.hpp>

namespace DDA {
  template <typename T>
  class MatVecGpu : public MatVec<T> {
    typedef T ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

    OpenCL::StubPool pool;

    GpuMatVec<ftype>& matVec;

    DipVector<ftype> argGpu;
    DipVector<ftype> resultGpu;

    cl::CommandQueue queue;

  public:
    MatVecGpu (const OpenCL::StubPool& pool, const cl::CommandQueue& queue, GpuMatVec<ftype>& matVec, OpenCL::VectorAccounting& accounting);
    virtual ~MatVecGpu ();

    virtual void setCoupleConstants (const boost::shared_ptr<const CoupleConstants<ftype> >& cc);

    const DDAParams<ftype>& ddaParams () const { return MatVec<T>::ddaParams(); }
    const DDAParams<ftype>& g () const { return MatVec<T>::ddaParams(); }
    const DipoleGeometry& dipoleGeometry () const { return ddaParams ().dipoleGeometry (); }

    virtual void apply (const std::vector<ctype>& arg, std::vector<ctype>& result, bool conj, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
  };

  CALL_MACRO_FOR_OPENCL_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, MatVecGpu)
}

#endif // !DDA_MATVECGPU_HPP_INCLUDED
