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

#include "DMatrixGpu.hpp"

#include <LinAlg/FFTWPlan.hpp>

#include <DDA/DMatrixCpu.hpp>

namespace DDA {
  // TODO: Implement DMatrix generation on GPU

  template <class T> void DMatrixGpu<T>::createDMatrix (UNUSED const OpenCL::StubPool& pool, const std::vector<cl::CommandQueue>& queues, const DDAParams<T>& ddaParams, const LinAlg::GpuFFTPlanFactory<T>& planFactory, OpenCL::MultiGpuVector<std::complex<T> >& dMatrix, const boost::shared_ptr<const Beam<T> >& beam) {
    ASSERT (dMatrix.vectorCount () == ddaParams.procs ());
    for (size_t i = 0; i < ddaParams.procs (); i++)
      ASSERT (dMatrix[i].size () == ddaParams.cgridY () * ddaParams.cgridZ () * ddaParams.localCGridX (i) * 6);
    (void) planFactory;
    boost::multi_array<Math::SymMatrix3<std::complex<T> >, 3> dMatrixCpu (boost::extents[ddaParams.gridY ()][ddaParams.gridZ ()][ddaParams.gridX ()], boost::fortran_storage_order ());
    DMatrixCpu<T>::createDMatrix (ddaParams, LinAlg::getFFTWPlanFactory<T> (), dMatrixCpu, beam);
    for (size_t i = 0; i < ddaParams.procs (); i++)
      dMatrix[i].write (queues[i], (const std::complex<T>*) (dMatrixCpu.data () + ddaParams.gridY () * ddaParams.gridZ () * ddaParams.localX0 (i)));
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, DMatrixGpu)
}
