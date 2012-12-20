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

#include "GpuIterativeSolver.hpp"

#include "GpuIterativeSolver.stub.hpp"

#include <LinAlg/GpuLinComb.hpp>

#include <DDA/Debug.hpp>

//#define INFO(x) Debug::info (#x, this->queues (), x)
#define INFO(x) do { } while (0)

namespace DDA {
  namespace {
    template <typename F> std::complex<F> vecProdConj (const std::vector<cl::CommandQueue>& queues, const DipVector<F>& v1, const DipVector<F>& v2) {
      ASSERT (v1.vectorCount () == v2.vectorCount ());

      std::complex<F> sum = 0;

      for (size_t j = 0; j < v1.vectorCount (); j++) {
        ASSERT (v1[j].size () == v2[j].size ());
        std::vector<std::complex<F> > i1 (v1[j].size ());
        std::vector<std::complex<F> > i2 (v2[j].size ());
        v1[j].read (queues[j], i1);
        v2[j].read (queues[j], i2);
      
        for (size_t i = 0; i < v1[j].size (); i++)
          sum += i1[i] * i2[i];
      }

      return sum;
    }
  }

  template <class F> GpuIterativeSolver<F>::GpuIterativeSolver (const OpenCL::StubPool& pool, const std::vector<cl::CommandQueue>& queues, const DDAParams<ftype>& ddaParams, GpuMatVec<ftype>& matVec, csize_t maxResIncrease, csize_t maxIter, OpenCL::VectorAccounting& accounting, Core::ProfilingDataPtr prof) :
    IterativeSolverBase<ftype> (ddaParams, maxResIncrease, maxIter),
    pool_ (pool),
    queues_ (queues),
    linComb (pool, queues, accounting, prof),
    matVec_ (matVec),
    Avecbuffer_ (pool, queues, g (), accounting, "Avecbuffer"),
    rvec_ (pool, queues, g (), accounting, "rvec"),
    xvec_ (pool, queues, g (), accounting, "xvec"),
    tmpVec1_ (pool, queues, g (), accounting, "tmpVec1"),
    tempVec (pool, 1, accounting, "temp")
  {
    pool.set (stub, prof);
    ASSERT (&ddaParams == &matVec.ddaParams ());
    ASSERT (queues.size () == ddaParams.procs ());
  }
  template <class F> GpuIterativeSolver<F>::~GpuIterativeSolver () {}

  template <class F> void GpuIterativeSolver<F>::setCoupleConstants (const boost::shared_ptr<const CoupleConstants<ftype> >& cc) {
    matVec ().setCoupleConstants (queues (), cc);
  }

  template <class F> F GpuIterativeSolver<F>::initGeneral (const std::vector<ctype>& einc, std::ostream& log, const std::vector<ctype>& start, UNUSED Core::ProfilingDataPtr prof) {
    DipVector<ftype>& pvec = tmpVec1 ();
    pvec.setToZero (queues ());

    pvec.write (queues (), einc);

    for (size_t i = 0; i < queues ().size (); i++)
      stub->setupPvec<ftype> (queues ()[i], OpenCL::getDefaultWorkItemCount (queues ()[i]), pvec[i], matVec ().materialsGpuG ()[i], matVec ().ccSqrtGpuG ()[i], g ().localCNvCount (i), g ().localCVecStride (i));

    linComb.reduce (queues (), pvec, tempVec.pointer ());
    ftype temp;
    tempVec.read (queues ()[0], &temp);
    this->residScale = 1 / temp;

    ftype inprodR;
    if (start.size () != 0) {
      DipVector<ftype>& xvec = this->xvec ();
      DipVector<ftype>& startGpu = this->xvec ();
      startGpu.write (queues (), start);
      for (size_t i = 0; i < queues ().size (); i++)
        stub->setupXvecStartValue<ftype> (queues ()[i], OpenCL::getDefaultWorkItemCount (queues ()[i]), xvec[i], startGpu[i], matVec ().materialsGpuG ()[i], matVec ().ccSqrtGpuG ()[i], g ().localCNvCount (i), g ().localCVecStride (i));
      DipVector<ftype>& Avecbuffer = this->Avecbuffer ();
      matVec ().apply (queues (), xvec, Avecbuffer, false);
      DipVector<ftype>& rvec = this->rvec ();
      linComb.linComb (queues (), Avecbuffer, linComb.minusOne, pvec, rvec);
      linComb.reduce (queues (), rvec, tempVec.pointer ());
      inprodR = tempVec.pointer ().read (queues ()[0]);
      log << "Use loaded start value" << std::endl;
    } else {

      DipVector<ftype>& Avecbuffer = this->Avecbuffer ();
      INFO (pvec);
      matVec ().apply (queues (), pvec, Avecbuffer, false);
      INFO (Avecbuffer);

      DipVector<ftype>& rvec = this->rvec ();
      linComb.linComb (queues (), Avecbuffer, linComb.minusOne, pvec, rvec);
      linComb.reduce (queues (), rvec, tempVec.pointer ());
      inprodR = tempVec.pointer ().read (queues ()[0]);
    
      log << "temp = " << temp << ", inprodR = " << inprodR << std::endl;

      DipVector<ftype>& xvec = this->xvec ();
      if (temp < inprodR) {
        log << "Use 0" << std::endl;
        xvec.setToZero (queues ());
        swap (rvec, pvec);
        inprodR = temp;
      } else {
        log << "Use pvec" << std::endl;
        swap (xvec, pvec);
      }
    }

    log << "|r_0|^2: " << temp << std::endl;
    return inprodR;
  }

  template <class F> boost::shared_ptr<std::vector<std::complex<F> > > GpuIterativeSolver<F>::getResult (UNUSED std::ostream& log, UNUSED Core::ProfilingDataPtr prof) {
    DipVector<ftype>& pvec = tmpVec1 ();
    const DipVector<ftype>& xvec = this->xvec ();

    for (size_t i = 0; i < queues ().size (); i++)
      stub->getResult<ftype> (queues ()[i], OpenCL::getDefaultWorkItemCount (queues ()[i]), pvec[i], matVec ().materialsGpuG ()[i], matVec ().ccSqrtGpuG ()[i], xvec[i], g ().localCNvCount (i), g ().localCVecStride (i));

    return pvec.read (queues ());
  }
  
  CALL_MACRO_FOR_OPENCL_FP_TYPES(CREATE_TEMPLATE_INSTANCE, GpuIterativeSolver)
}
