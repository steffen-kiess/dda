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

#include "GpuMatVec.hpp"

#include "GpuMatVec.stub.hpp"

#include <DDA/GpuTransposePlan.hpp>
#include <DDA/Debug.hpp>

namespace DDA {
  //#define INFO(x) Debug::info (#x, queues, x)
#define INFO(x) do { } while (0)

  namespace {
    template <class F> std::vector<size_t> getNvCount (const DDAParams<F>& ddaParams) {
      std::vector<size_t> res (ddaParams.procs () ());
      for (size_t i = 0; i < ddaParams.procs (); i++)
        res[i] = ddaParams.localNvCount (i);
      return res;
    }
    template <class F> std::vector<size_t> getVecSize (const DDAParams<F>& ddaParams) {
      std::vector<size_t> res (ddaParams.procs () ());
      for (size_t i = 0; i < ddaParams.procs (); i++)
        res[i] = ddaParams.localVecSize (i);
      return res;
    }
    template <class F> std::vector<size_t> getDMSize (const DDAParams<F>& ddaParams, size_t slicesCount) {
      std::vector<size_t> res (ddaParams.procs () ());
      for (size_t i = 0; i < ddaParams.procs (); i++)
        res[i] = (ddaParams.cgridY () * ddaParams.cgridZ () * /*ddaParams.cgridX ()*/slicesCount * 6) ();
      return res;
    }
    template <class F> std::vector<size_t> getXMSize (const DDAParams<F>& ddaParams) {
      std::vector<size_t> res (ddaParams.procs () ());
      for (size_t i = 0; i < ddaParams.procs (); i++)
        res[i] = (std::max (ddaParams.cgridX () * ddaParams.dipoleGeometry ().box ().y () * ddaParams.localCBoxZ (i) * 3, ddaParams.localCGridX (i) * ddaParams.dipoleGeometry ().box ().y () * ddaParams.dipoleGeometry ().box ().z () * 3)) ();
      return res;
    }
    template <class F> std::vector<size_t> getXMBSize (const DDAParams<F>& ddaParams) {
      std::vector<size_t> res (ddaParams.procs () ());
      for (size_t i = 0; i < ddaParams.procs (); i++)
        res[i] = ddaParams.localBlocksSize (i);
      return res;
    }
    template <class F> std::vector<size_t> getSlicesSize (const DDAParams<F>& ddaParams, size_t slicesCount) {
      std::vector<size_t> res (ddaParams.procs () ());
      for (size_t i = 0; i < ddaParams.procs (); i++)
        res[i] = (ddaParams.cgridZ () * ddaParams.cgridY () * 3 * slicesCount) ();
      // ddaParams.cgridY () * ddaParams.cgridZ () * 3 * slicesCount
      return res;
    }
    template <class F> std::vector<size_t> getZero (const DDAParams<F>& ddaParams) {
      std::vector<size_t> res (ddaParams.procs () ());
      for (size_t i = 0; i < ddaParams.procs (); i++)
        res[i] = 0;
      return res;
    }
  }

  template <class F> boost::shared_ptr<GpuMatVec<F> > GpuMatVec<F>::create (const std::vector<cl::CommandQueue>& queues, const DDAParams<ftype>& ddaParams, const boost::const_multi_array_ref<Math::SymMatrix3<ctype>, 3>& Dmatrix, const LinAlg::GpuFFTPlanFactory<ftype>& gpuPlanFactory, const OpenCL::StubPool& pool, OpenCL::VectorAccounting& accounting, Core::ProfilingDataPtr prof) {
    return boost::shared_ptr<GpuMatVec> (new GpuMatVec (queues, ddaParams, &Dmatrix, NULL, gpuPlanFactory, pool, accounting, prof));
  }

  template <class F> boost::shared_ptr<GpuMatVec<F> > GpuMatVec<F>::create (const std::vector<cl::CommandQueue>& queues, const DDAParams<ftype>& ddaParams, const OpenCL::MultiGpuVector<ctype>& dMatrixGpu, const LinAlg::GpuFFTPlanFactory<ftype>& gpuPlanFactory, const OpenCL::StubPool& pool, OpenCL::VectorAccounting& accounting, Core::ProfilingDataPtr prof) {
    return boost::shared_ptr<GpuMatVec> (new GpuMatVec (queues, ddaParams, NULL, &dMatrixGpu, gpuPlanFactory, pool, accounting, prof));
  }

  template <class F> GpuMatVec<F>::GpuMatVec (const std::vector<cl::CommandQueue>& queues, const DDAParams<ftype>& ddaParams, const boost::const_multi_array_ref<Math::SymMatrix3<ctype>, 3>* dMatrixCpu, const OpenCL::MultiGpuVector<ctype>* dMatrixGpu, const LinAlg::GpuFFTPlanFactory<ftype>& gpuPlanFactory, const OpenCL::StubPool& pool, OpenCL::VectorAccounting& accounting, Core::ProfilingDataPtr prof) :
    memoryAccessSize (32 * sizeof (F)),
    slicesCount (memoryAccessSize / sizeof (ctype) * 4),
    ddaParams_ (ddaParams),
    pool (pool),
    options (pool.options ()),
    materialsGpu (pool, queues, getNvCount (g ()), accounting, "materials"),
    positionsGpu (pool, queues, g (), accounting, "positions"),
    ccSqrtGpuSet (false),
    ccSqrtGpu (pool, queues, g ().dipoleGeometry ().materials ().size () * 3, accounting, "ccSqrt"),
    dMatrixCpu (dMatrixCpu),
    dMatrixGpuInst (dMatrixCpu ? new OpenCL::MultiGpuVector<ctype> (pool, queues, getDMSize (g (), slicesCount), accounting, "dMatrixSlice") : NULL),
    dMatrixGpu (dMatrixCpu ? *dMatrixGpuInst : *dMatrixGpu),
    xMatrixGpu (pool, queues, getXMSize (g ()), accounting, "xMatrix"),
    xMatrixGpu2 (pool, queues, g ().procs () > 1 ? getXMBSize (g ()) : getZero (g ()), accounting, "xMatrix2"),
    xMatrixCpu (g ().procs () > 1 ? (g ().cgridX () * g ().dipoleGeometry ().box ().y () * g ().dipoleGeometry ().box ().z () * 3) () : 0),
    slicesGpu (pool, queues, getSlicesSize (g (), slicesCount), accounting, "slices"),
    slicesTrGpu (pool, queues, getSlicesSize (g (), slicesCount), accounting, "slicesTr"),
    planX (g ().procs () ()),
    planZFull (g ().procs () ()),
    planZLast (g ().procs () ()),
    planYFull (g ().procs () ()),
    planYLast (g ().procs () ())
  {
    pool.set (stub, prof);
    //Core::OStream::getStderr () << "D-C, slicesCount = " << slicesCount << ", gridX = " << g ().gridX () << std::endl;

    ASSERT ((slicesCount & (slicesCount - 1)) == 0); // slicesCount must be POT
    for (size_t i = 0; i < g ().procs (); i++)
      materialsGpu[i].write (queues[i], g ().dipoleGeometry ().materialIndices ().data () +  g ().localVec0 (i), 0, g ().localNvCount (i));
    {
      std::vector<uint32_t> pos (g ().vecSize ());
      for (int i = 0; i < 3; i++)
        for (uint32_t j = 0; j < g ().cnvCount (); j++)
          pos[j + i * g ().vecStride ()] = g ().dipoleGeometry ().getGridCoordinates (j)[i];
      positionsGpu.write (queues, pos);
    }
    for (size_t i = 0; i < g ().procs (); i++) {
      planX[i] = gpuPlanFactory.createPlan (pool, queues[i].getInfo<CL_QUEUE_DEVICE> (), g ().cgridX (), g ().dipoleGeometry ().box ().y () * g ().localCBoxZ (i) * 3, true, false, true, true, accounting);
      // times = 3, stride = gridY * gridZ
      // Do the entire slice at once. This means a useless FFT on
      // the padding is done, but there are fewer calls to the FFT plan
      planZFull[i] = gpuPlanFactory.createPlan (pool, queues[i].getInfo<CL_QUEUE_DEVICE> (), g ().cgridZ (), g ().cgridY () * 3 * slicesCount, true, false, true, true, accounting);
      planZLast[i] = gpuPlanFactory.createPlan (pool, queues[i].getInfo<CL_QUEUE_DEVICE> (), g ().cgridZ (), g ().cgridY () * 3 * (g ().localGridX (i) % slicesCount), true, false, true, true, accounting);
      // times = 1
      planYFull[i] = gpuPlanFactory.createPlan (pool, queues[i].getInfo<CL_QUEUE_DEVICE> (), g ().cgridY (), g ().cgridZ () * 3 * slicesCount, true, false, true, true, accounting);
      planYLast[i] = gpuPlanFactory.createPlan (pool, queues[i].getInfo<CL_QUEUE_DEVICE> (), g ().cgridY (), g ().cgridZ () * 3 * (g ().localGridX (i) % slicesCount), true, false, true, true, accounting);
    }
  }
  template <class F> GpuMatVec<F>::~GpuMatVec () {}

  template <class F> void GpuMatVec<F>::setCoupleConstants (const std::vector<cl::CommandQueue>& queues, const boost::shared_ptr<const CoupleConstants<ftype> >& cc) {
    ccSqrtGpu.writeCopies (queues, (const ctype*) cc->cc_sqrt ().data ());
    ccSqrtGpuSet = true;
  }

  template <class F> void GpuMatVec<F>::apply (const std::vector<cl::CommandQueue>& queues, const DipVector<ftype>& argGpu, DipVector<ftype>& resultGpu, bool conj, Core::ProfilingDataPtr prof) {
    const DDAParams<ftype>& g = this->ddaParams ();

    ASSERT (ccSqrtGpuSet);

    if (options.enableSync ()) {
      Core::ProfileHandle _p (prof, "i");
      for (size_t i = 0; i < g.procs (); i++)
        queues[i].finish ();
    }

    {
      Core::ProfileHandle _p (prof, "initXMatrix");
      xMatrixGpu.setToZero (queues);
      for (size_t i = 0; i < g.procs (); i++)
        stub->initXMatrix<ftype> (queues[i], OpenCL::getDefaultWorkItemCount (queues[i]), xMatrixGpu[i], materialsGpu[i], positionsGpu[i], ccSqrtGpu[i], argGpu[i], conj, g.localCZ0 (i), g.gridX (), g.dipoleGeometry ().box ().y (), g.localCBoxZ (i), g.localCNvCount (i), g.localCVecStride (i));
      if (options.enableSync ()) {
        //Core::ProfileHandle _p (prof, "s");
        for (size_t i = 0; i < g.procs (); i++)
          queues[i].finish ();
      }
    }

    //for (size_t j = 0; j < g.dipoleGeometry ().box ().z () * 3; j++) {
    { size_t j = 0;
      Core::ProfileHandle _p1 (prof, "fft" /* "planXf" */);
      for (size_t i = 0; i < g.procs (); i++)
        planX[i]->fftInPlace (queues[i], xMatrixGpu[i], g.dipoleGeometry ().box ().y () * g.cgridX () * j);
    }

    if (g.procs () != 1) {
      if (options.enableSync ()) {
        Core::ProfileHandle _p (prof, "trans1_s");
        for (size_t i = 0; i < g.procs (); i++)
          queues[i].finish ();
      }
      Core::ProfileHandle _p (prof, "trans1");
      for (size_t i = 0; i < g.procs (); i++)
        for (size_t j = 0; j < g.procs (); j++)
          GpuTransposePlan<ctype>
            (pool,
             GpuTransposeDimension (g.localCGridX (j), 1, 1),
             GpuTransposeDimension (g.dipoleGeometry ().box ().y (), g.cgridX (), g.localCGridX (j)),
             GpuTransposeDimension (g.localCBoxZ (i), g.cgridX () * g.dipoleGeometry ().box ().y (), g.localCGridX (j) * g.dipoleGeometry ().box ().y ()),
             GpuTransposeDimension (3, g.cgridX () * g.dipoleGeometry ().box ().y () * g.localCBoxZ (i), g.localCGridX (j) * g.dipoleGeometry ().box ().y () * g.localCBoxZ (i))
             ).transpose (queues[i],
                          xMatrixGpu[i], g.localX0 (j),
                          xMatrixGpu2[i], g.localCBlockOffset (i, j), prof);
      for (size_t i = 0; i < g.procs (); i++) {
        for (size_t j = i + 1; j < g.procs (); j++) {
          // Exchange (i, j) and (j, i)
          csize_t size1 = g.localCBlockOffset (i, j + 1) - g.localCBlockOffset (i, j); // block i->j
          csize_t size2 = g.localCBlockOffset (j, i + 1) - g.localCBlockOffset (j, i); // block j->i
          ASSERT (size1 == size2);

          // GPU=>CPU
          ASSERT (size1 + size2 <= xMatrixCpu.size ());
          xMatrixGpu2[i].read (queues[i], xMatrixCpu.data (), g.localCBlockOffset (i, j), size1);
          xMatrixGpu2[j].read (queues[j], xMatrixCpu.data () + size1 (), g.localCBlockOffset (j, i), size2);

          // CPU=>GPU
          xMatrixGpu2[i].write (queues[i], xMatrixCpu.data () + size1 (), g.localCBlockOffset (i, j), size2);
          xMatrixGpu2[j].write (queues[j], xMatrixCpu.data (), g.localCBlockOffset (j, i), size1);
        }
      }
      for (size_t i = 0; i < g.procs (); i++)
        for (size_t j = 0; j < g.procs (); j++)
          for (size_t comp = 0; comp < 3; comp++)
            queues[i].enqueueCopyBuffer (xMatrixGpu2[i].getData (), 
                                         xMatrixGpu[i].getDataWritable (),
                                         ((g.localCBlockOffset (i, j) + g.localCGridX (i) * g.dipoleGeometry ().box ().y () * g.localCBoxZ (j) * comp) * sizeof (ctype)) (),
                                         ((g.localCGridX (i) * g.dipoleGeometry ().box ().y () * (g.localCZ0 (j) + g.dipoleGeometry ().box ().z () * comp)) * sizeof (ctype)) (),
                                         ((g.localCGridX (i) * g.dipoleGeometry ().box ().y () * g.localCBoxZ (j)) * sizeof (ctype)) ());
      if (options.enableSync ()) {
        for (size_t i = 0; i < g.procs (); i++)
          queues[i].finish ();
      }
    }

    std::vector<size_t> slicesCountCur (g.procs () ());
    {
      Core::ProfileHandle _p_ (prof, "il");
      for (size_t si = 0; si < g.localCGridXMax (); si+= slicesCount) {
        //csize_t slicesCountCur = si + slicesCount > g.cgridX () ? g.cgridX () - si : slicesCount;
        for (size_t i = 0; i < g.procs (); i++)
          slicesCountCur[i] = (si >= g.localCGridX (i) ? cuint32_t (0) : (si + slicesCount > g.localCGridX (i) ? g.localCGridX (i) () - si : slicesCount)) ();
        if (dMatrixCpu) {
          for (size_t i = 0; i < g.procs (); i++)
            (*dMatrixGpuInst)[i].write (queues[i], (const ctype*) dMatrixCpu->data () + g.gridY () * g.gridZ () * (si + g.localX0 (i)) * 6, 0, g.gridY () * g.gridZ () * slicesCountCur[i] * 6);
        }
        slicesGpu.setToZero (queues);
        {
          Core::ProfileHandle _p (prof, "tr");
          Core::ProfileHandle _p2 (prof, "1");
          for (size_t i = 0; i < g.procs (); i++)
            GpuTransposePlan<ctype>
              (pool,
               GpuTransposeDimension (slicesCountCur[i], 1, g.cgridZ () * g.cgridY () * 3),
               GpuTransposeDimension (g.dipoleGeometry ().box ().y (), g.localCGridX (i), g.cgridZ ()),
               GpuTransposeDimension (g.dipoleGeometry ().box ().z (), g.localCGridX (i) * g.dipoleGeometry ().box ().y (), 1),
               GpuTransposeDimension (3, g.localCGridX (i) * g.dipoleGeometry ().box ().y () * g.dipoleGeometry ().box ().z (), g.cgridZ () * g.cgridY ())
               ).transpose (queues[i],
                            xMatrixGpu[i], si,
                            slicesGpu[i], 0, prof);
        }
        //for (size_t i = si; i < g.cgridX () && i < si + slicesCount; i++) {
        //csize_t offset = g.cgridZ () * g.cgridY () * 3 * (i - si);
        for (size_t i = 0; i < g.procs (); i++) {
          //for (size_t j = 0; j < slicesCountCur[i]; j++) {
          //for (size_t comp = 0; comp < 3; comp++) {
          { size_t comp = 0;
            Core::ProfileHandle _p1 (prof, "fft" /* "planZf" */);
            if (slicesCountCur[i] == slicesCount) {
              planZFull[i]->fftInPlace (queues[i], slicesGpu[i], comp * g.gridY () * g.gridZ ());
            } else {
              ASSERT (slicesCountCur[i] == g.localGridX (i) % slicesCount);
              planZLast[i]->fftInPlace (queues[i], slicesGpu[i], comp * g.gridY () * g.gridZ ());
            }
          }
          //}
        }
        {
          Core::ProfileHandle _p (prof, "tr");
          Core::ProfileHandle _p2 (prof, "2");
          for (size_t i = 0; i < g.procs (); i++)
            GpuTransposePlan<ctype>
              (pool,
               GpuTransposeDimension (g.cgridZ (), 1, g.cgridY ()),
               GpuTransposeDimension (g.cgridY (), g.cgridZ (), 1),
               GpuTransposeDimension (3, g.cgridZ () * g.cgridY (), g.cgridZ () * g.cgridY ()),
               GpuTransposeDimension (slicesCountCur[i], g.cgridZ () * g.cgridY () * 3, g.cgridZ () * g.cgridY () * 3)
               ).transpose (queues[i],
                            slicesGpu[i], 0/*offset*/,
                            slicesTrGpu[i], 0, prof);
        }
        for (size_t i = 0; i < g.procs (); i++) {
          //for (size_t j = 0; j < slicesCountCur[i]; j++) {
          {
            Core::ProfileHandle _p1 (prof, "fft" /* "planYf" */);
            if (slicesCountCur[i] == slicesCount) {
              planYFull[i]->fftInPlace (queues[i], slicesTrGpu[i], 0);
            } else {
              ASSERT (slicesCountCur[i] == g.localGridX (i) % slicesCount);
              planYLast[i]->fftInPlace (queues[i], slicesTrGpu[i], 0);
            }
          }
          //}
        }
        {
          Core::ProfileHandle _p1 (prof, "iil");
          for (size_t i = 0; i < g.procs (); i++)
            stub->innerLoop<ftype> (queues[i],
                                    cl::NDRange (g.gridY (), g.gridZ (), slicesCountCur [i]),
                                    slicesTrGpu[i], dMatrixGpu[i],
                                    dMatrixCpu ? 0 : cuint32_t (si /*i*/),
                                    g.gridX (), g.gridY (), g.gridZ ());
          if (options.enableSync ()) {
            //Core::ProfileHandle _p (prof, "s");
            for (size_t i = 0; i < g.procs (); i++)
              queues[i].finish ();
          }
        }
        for (size_t i = 0; i < g.procs (); i++) {
          //for (size_t j = 0; j < slicesCountCur[i]; j++) {
          {
            Core::ProfileHandle _p1 (prof, "fft" /* "planYb" */);
            if (slicesCountCur[i] == slicesCount) {
              planYFull[i]->ifftInPlace (queues[i], slicesTrGpu[i], 0);
            } else {
              ASSERT (slicesCountCur[i] == g.localGridX (i) % slicesCount);
              planYLast[i]->ifftInPlace (queues[i], slicesTrGpu[i], 0);
            }
          }
          //}
        }
        {
          Core::ProfileHandle _p (prof, "tr");
          Core::ProfileHandle _p2 (prof, "3");
          for (size_t i = 0; i < g.procs (); i++)
            GpuTransposePlan<ctype>
              (pool,
               GpuTransposeDimension (g.cgridZ (), g.cgridY (), 1),
               GpuTransposeDimension (g.cgridY (), 1, g.cgridZ ()),
               GpuTransposeDimension (3, g.cgridZ () * g.cgridY (), g.cgridZ () * g.cgridY ()),
               GpuTransposeDimension (slicesCountCur[i], g.cgridZ () * g.cgridY () * 3, g.cgridZ () * g.cgridY () * 3)
               ).transpose (queues[i],
                            slicesTrGpu[i], 0,
                            slicesGpu[i], 0 /*offset*/, prof);
        }
        for (size_t i = 0; i < g.procs (); i++) {
          //for (size_t j = 0; j < slicesCountCur[i]; j++) {
          //for (size_t comp = 0; comp < 3; comp++) {
          { // size_t comp = 0;
            Core::ProfileHandle _p1 (prof, "fft" /* "planZb" */);
            if (slicesCountCur[i] == slicesCount) {
              planZFull[i]->ifftInPlace (queues[i], slicesGpu[i], 0);
            } else {
              ASSERT (slicesCountCur[i] == g.localGridX (i) % slicesCount);
              planZLast[i]->ifftInPlace (queues[i], slicesGpu[i], 0);
            }
          }
          //}
        }
        //}
        {
          Core::ProfileHandle _p (prof, "tr");
          Core::ProfileHandle _p2 (prof, "4");
          for (size_t i = 0; i < g.procs (); i++)
            GpuTransposePlan<ctype>
              (pool,
               GpuTransposeDimension (slicesCountCur[i], g.cgridZ () * g.cgridY () * 3, 1),
               GpuTransposeDimension (g.dipoleGeometry ().box ().y (), g.cgridZ (), g.localCGridX (i)),
               GpuTransposeDimension (g.dipoleGeometry ().box ().z (), 1, g.localCGridX (i) * g.dipoleGeometry ().box ().y ()),
               GpuTransposeDimension (3, g.cgridZ () * g.cgridY (), g.localCGridX (i) * g.dipoleGeometry ().box ().y () * g.dipoleGeometry ().box ().z ())
               ).transpose (queues[i],
                            slicesGpu[i], 0,
                            xMatrixGpu[i], si, prof);
        }
      }
    }

    if (g.procs () != 1) {
      if (options.enableSync ()) {
        Core::ProfileHandle _p (prof, "trans2_s");
        for (size_t i = 0; i < g.procs (); i++)
          queues[i].finish ();
      }
      Core::ProfileHandle _p (prof, "trans2");
      for (size_t i = 0; i < g.procs (); i++)
        for (size_t j = 0; j < g.procs (); j++)
          for (size_t comp = 0; comp < 3; comp++)
            queues[i].enqueueCopyBuffer (xMatrixGpu[i].getData (), 
                                         xMatrixGpu2[i].getDataWritable (),
                                         ((g.localCGridX (i) * g.dipoleGeometry ().box ().y () * (g.localCZ0 (j) + g.dipoleGeometry ().box ().z () * comp)) * sizeof (ctype)) (),
                                         ((g.localCBlockOffset (i, j) + g.localCGridX (i) * g.dipoleGeometry ().box ().y () * g.localCBoxZ (j) * comp) * sizeof (ctype)) (),
                                         ((g.localCGridX (i) * g.dipoleGeometry ().box ().y () * g.localCBoxZ (j)) * sizeof (ctype)) ());
      for (size_t i = 0; i < g.procs (); i++) {
        for (size_t j = i + 1; j < g.procs (); j++) {
          // Exchange (i, j) and (j, i)
          csize_t size1 = g.localCBlockOffset (i, j + 1) - g.localCBlockOffset (i, j); // block i->j
          csize_t size2 = g.localCBlockOffset (j, i + 1) - g.localCBlockOffset (j, i); // block j->i
          ASSERT (size1 == size2);

          // GPU=>CPU
          ASSERT (size1 + size2 <= xMatrixCpu.size ());
          xMatrixGpu2[i].read (queues[i], xMatrixCpu.data (), g.localCBlockOffset (i, j), size1);
          xMatrixGpu2[j].read (queues[j], xMatrixCpu.data () + size1 (), g.localCBlockOffset (j, i), size2);

          // CPU=>GPU
          xMatrixGpu2[i].write (queues[i], xMatrixCpu.data () + size1 (), g.localCBlockOffset (i, j), size2);
          xMatrixGpu2[j].write (queues[j], xMatrixCpu.data (), g.localCBlockOffset (j, i), size1);
        }
      }
      for (size_t i = 0; i < g.procs (); i++)
        for (size_t j = 0; j < g.procs (); j++)
          GpuTransposePlan<ctype>
            (pool,
             GpuTransposeDimension (g.localCGridX (j), 1, 1),
             GpuTransposeDimension (g.dipoleGeometry ().box ().y (), g.localCGridX (j), g.cgridX ()),
             GpuTransposeDimension (g.localCBoxZ (i), g.localCGridX (j) * g.dipoleGeometry ().box ().y (), g.cgridX () * g.dipoleGeometry ().box ().y ()),
             GpuTransposeDimension (3, g.localCGridX (j) * g.dipoleGeometry ().box ().y () * g.localCBoxZ (i), g.cgridX () * g.dipoleGeometry ().box ().y () * g.localCBoxZ (i))
             ).transpose (queues[i],
                          xMatrixGpu2[i], g.localCBlockOffset (i, j),
                          xMatrixGpu[i], g.localX0 (j), prof);
      if (options.enableSync ()) {
        for (size_t i = 0; i < g.procs (); i++)
          queues[i].finish ();
      }
    }

    //for (size_t j = 0; j < g.dipoleGeometry ().box ().z () * 3; j++) {
    { size_t j = 0;
      Core::ProfileHandle _p1 (prof, "fft" /* "planXb" */);
      for (size_t i = 0; i < g.procs (); i++)
        planX[i]->ifftInPlace (queues[i], xMatrixGpu[i], g.dipoleGeometry ().box ().y () * g.cgridX () * j);
    }

    {
      Core::ProfileHandle _p1 (prof, "createResVec");
      for (size_t i = 0; i < g.procs (); i++)
        stub->createResVec<ftype> (queues[i], OpenCL::getDefaultWorkItemCount (queues[i]), xMatrixGpu[i], materialsGpu[i], positionsGpu[i], ccSqrtGpu[i], argGpu[i], resultGpu[i], conj, g.localCZ0 (i), g.gridX (), g.dipoleGeometry ().box ().y (), g.localCBoxZ (i), g.localCNvCount (i), g.localCVecStride (i));
      if (options.enableSync ()) {
        //Core::ProfileHandle _p (prof, "s");
        for (size_t i = 0; i < g.procs (); i++)
          queues[i].finish ();
      }
    }
  }

  CALL_MACRO_FOR_OPENCL_FP_TYPES(CREATE_TEMPLATE_INSTANCE, GpuMatVec)
}
