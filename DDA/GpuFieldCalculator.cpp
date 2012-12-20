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

#include "GpuFieldCalculator.hpp"

#include "GpuFieldCalculator.stub.hpp"

#include <Math/Vector3.hpp>

#include <DDA/DDAParams.hpp>

namespace DDA {
  template <class T>
  GpuFieldCalculator<T>::GpuFieldCalculator (const OpenCL::StubPool& pool, OpenCL::VectorAccounting& accounting, const DDAParams<ftype>& ddaParams, Core::ProfilingDataPtr prof)
    : FieldCalculator<ftype> (ddaParams),
      stub (pool.get<GpuFieldCalculatorStub> (prof)),
      positions (pool, ddaParams.cvecSize (), accounting, "GpuFieldCalculator.positions"),
      valid (pool, ddaParams.cnvCount (), accounting, "GpuFieldCalculator.valid"),
      pvec (pool, ddaParams.cvecSize (), accounting, "GpuFieldCalculator.pvec"),
      device (stub->context ().getInfo<CL_CONTEXT_DEVICES> ()[0]),
      queue (cl::CommandQueue (stub->context (), device, 0)),
      workGroups (device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS> () * 8),
      gpuRes (pool, workGroups * 3, accounting, "GpuFieldCalculator.gpuRes"),
      cpuRes (workGroups * 3)
  {
    {
      std::vector<uint32_t> pos (ddaParams.vecSize ());
      for (int i = 0; i < 3; i++)
        for (uint32_t j = 0; j < ddaParams.cnvCount (); j++)
          pos[j + i * ddaParams.vecStride ()] = ddaParams.dipoleGeometry ().getGridCoordinates (j)[i];
      positions.write (queue, pos);
    }
    valid.write (queue, ddaParams.dipoleGeometry ().valid ().data ());
  }

  template <class T>
  GpuFieldCalculator<T>::~GpuFieldCalculator () {}

  template <class T>
  void GpuFieldCalculator<T>::setPVec (const std::vector<ctype>& pvec) {
    this->pvec.write (queue, pvec);
  }

  template <class T>
  Math::Vector3<std::complex<T> > GpuFieldCalculator<T>::calcField (Math::Vector3<ftype> n) {
    stub->calcField<T> (queue, workGroups * 32, 32, valid, positions, pvec, ddaParams ().nvCount (), ddaParams ().vecStride (), gpuRes, ddaParams ().kd (), n.x (), n.y (), n.z ());
    gpuRes.read (queue, cpuRes.data ());
    Math::Vector3<ctype> sum (0, 0, 0);
    for (size_t i = 0; i < workGroups; i++)
      sum += Math::Vector3<ctype> (cpuRes[i], cpuRes[i + workGroups], cpuRes[i + 2 * workGroups]);
    Math::Vector3<ctype> tbuff = sum - n * (n * sum);
    return (ctype (0, std::pow (ddaParams ().waveNum (), FPConst<ftype>::two)) * std::polar<ftype> (1, -ddaParams ().waveNum () * (static_cast<Math::Vector3<ftype> > (ddaParams ().dipoleGeometry ().origin ()) * n))) * tbuff;
  }

  CALL_MACRO_FOR_OPENCL_FP_TYPES (CREATE_TEMPLATE_INSTANCE, GpuFieldCalculator)
}
