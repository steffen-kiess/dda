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

#ifndef DDA_GPUMATVEC_HPP_INCLUDED
#define DDA_GPUMATVEC_HPP_INCLUDED

// matrix-vector-product implementation for the GPU

#include <Core/Profiling.hpp>

#include <OpenCL/Bindings.hpp>
#include <OpenCL/Vector.hpp>
#include <OpenCL/StubPool.hpp>

#include <LinAlg/GpuFFTPlan.hpp>

#include <DDA/DDAParams.hpp>
#include <DDA/DipVector.hpp>

#include <boost/static_assert.hpp>

namespace DDA {
  template <typename T>
  class GpuMatVec {
    typedef T ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

    const size_t memoryAccessSize;
    const size_t slicesCount;

    const DDAParams<ftype>& ddaParams_;

    OpenCL::StubPool pool;
    boost::shared_ptr<class GpuMatVecStub> stub;
    OpenCL::Options options;

    // Data structures and FFT plans for matVec
    OpenCL::MultiGpuVector<uint8_t> materialsGpu;
    DipVector<ftype, uint32_t> positionsGpu;
    bool ccSqrtGpuSet;
    OpenCL::MultiGpuVector<ctype> ccSqrtGpu;

    const boost::const_multi_array_ref<Math::SymMatrix3<ctype>, 3>* dMatrixCpu;
    boost::scoped_ptr<OpenCL::MultiGpuVector<ctype> > dMatrixGpuInst;
    const OpenCL::MultiGpuVector<ctype>& dMatrixGpu;
    OpenCL::MultiGpuVector<ctype> xMatrixGpu;
    OpenCL::MultiGpuVector<ctype> xMatrixGpu2;
    std::vector<ctype> xMatrixCpu;
    OpenCL::MultiGpuVector<ctype> slicesGpu;
    OpenCL::MultiGpuVector<ctype> slicesTrGpu;

    // times = Xmatrix.sizeZ () * 3, stride = Xmatrix.sizeY * Xmatrix.sizeX
    std::vector<boost::shared_ptr<LinAlg::GpuFFTPlan<ftype> > > planX;
    //// times = 3, stride = gridY * gridZ
    // times = 1
    std::vector<boost::shared_ptr<LinAlg::GpuFFTPlan<ftype> > > planZFull;
    std::vector<boost::shared_ptr<LinAlg::GpuFFTPlan<ftype> > > planZLast;
    // times = 1
    std::vector<boost::shared_ptr<LinAlg::GpuFFTPlan<ftype> > > planYFull;
    std::vector<boost::shared_ptr<LinAlg::GpuFFTPlan<ftype> > > planYLast;

    GpuMatVec (const std::vector<cl::CommandQueue>& queues, const DDAParams<ftype>& ddaParams, const boost::const_multi_array_ref<Math::SymMatrix3<ctype>, 3>* dMatrixCpu, const OpenCL::MultiGpuVector<ctype>* dMatrixGpu, const LinAlg::GpuFFTPlanFactory<ftype>& gpuPlanFactory, const OpenCL::StubPool& pool, OpenCL::VectorAccounting& accounting, Core::ProfilingDataPtr prof);

  public:
    static boost::shared_ptr<GpuMatVec> create (const std::vector<cl::CommandQueue>& queues, const DDAParams<ftype>& ddaParams, const boost::const_multi_array_ref<Math::SymMatrix3<ctype>, 3>& Dmatrix, const LinAlg::GpuFFTPlanFactory<ftype>& gpuPlanFactory, const OpenCL::StubPool& pool, OpenCL::VectorAccounting& accounting, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
    static boost::shared_ptr<GpuMatVec> create (const std::vector<cl::CommandQueue>& queues, const DDAParams<ftype>& ddaParams, const OpenCL::MultiGpuVector<ctype>& dMatrixGpu, const LinAlg::GpuFFTPlanFactory<ftype>& gpuPlanFactory, const OpenCL::StubPool& pool, OpenCL::VectorAccounting& accounting, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
    ~GpuMatVec ();

    void setCoupleConstants (const std::vector<cl::CommandQueue>& queues, const boost::shared_ptr<const CoupleConstants<ftype> >& cc);

    const DDAParams<ftype>& ddaParams () const { return ddaParams_; }
    const DDAParams<ftype>& g () const { return ddaParams_; }
    const Geometry& geometry () const { return ddaParams ().geometry (); }

    const OpenCL::MultiGpuVector<uint8_t>& materialsGpuG () { return materialsGpu; }
    const OpenCL::MultiGpuVector<uint32_t>& positionsGpuG () { return positionsGpu; }
    const OpenCL::MultiGpuVector<ctype>& ccSqrtGpuG () const { return ccSqrtGpu; }
  
    void apply (const std::vector<cl::CommandQueue>& queue, const DipVector<ftype>& arg, DipVector<ftype>& result, bool conj, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
  };

  CALL_MACRO_FOR_OPENCL_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, GpuMatVec)
}

#endif // !DDA_GPUMATVEC_HPP_INCLUDED
