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

#ifndef DDA_GPUITERATIVESOLVER_HPP_INCLUDED
#define DDA_GPUITERATIVESOLVER_HPP_INCLUDED

// Base class for iterative solvers running on the GPU

#include <Core/Profiling.hpp>

#include <OpenCL/Vector.hpp>
#include <OpenCL/StubPool.hpp>

#include <LinAlg/MultiGpuLinComb.hpp>

#include <DDA/DipVector.hpp>
#include <DDA/IterativeSolverBase.hpp>
#include <DDA/GpuMatVec.hpp>

namespace DDA {
  template <typename T>
  class GpuIterativeSolver : public IterativeSolverBase<T> {
    typedef T ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

    OpenCL::StubPool pool_;
    std::vector<cl::CommandQueue> queues_;
  protected:
    LinAlg::MultiGpuLinComb<ftype> linComb;
  private:
    boost::shared_ptr<class GpuIterativeSolverStub> stub;

    GpuMatVec<ftype>& matVec_;

    DipVector<ftype> Avecbuffer_;
    DipVector<ftype> rvec_;
    DipVector<ftype> xvec_;
    DipVector<ftype> tmpVec1_;

    OpenCL::Vector<ftype> tempVec;

  public:
    GpuIterativeSolver (const OpenCL::StubPool& pool, const std::vector<cl::CommandQueue>& queues, const DDAParams<ftype>& ddaParams, GpuMatVec<ftype>& matVec, csize_t maxResIncrease, csize_t maxIter, OpenCL::VectorAccounting& accounting, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
    virtual ~GpuIterativeSolver ();

    using IterativeSolverBase<T>::g;
    using IterativeSolverBase<T>::dipoleGeometry;
    GpuMatVec<ftype>& matVec () { return matVec_; }

    const std::vector<cl::CommandQueue>& queues () { return queues_; }

    virtual void setCoupleConstants (const boost::shared_ptr<const CoupleConstants<ftype> >& cc);

  protected:
    DipVector<ftype>& Avecbuffer () { return Avecbuffer_; }
    DipVector<ftype>& rvec () { return rvec_; }
    DipVector<ftype>& xvec () { return xvec_; }
    DipVector<ftype>& tmpVec1 () { return tmpVec1_; }

    virtual ftype initGeneral (const std::vector<ctype>& einc, std::ostream& log, const std::vector<ctype>& start, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
    virtual boost::shared_ptr<std::vector<ctype> > getResult (std::ostream& log, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ());
  };

  CALL_MACRO_FOR_OPENCL_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, GpuIterativeSolver)
}

#endif // !DDA_GPUITERATIVESOLVER_HPP_INCLUDED
