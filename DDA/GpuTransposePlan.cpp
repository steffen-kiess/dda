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

#include "GpuTransposePlan.hpp"

#include <Core/OStream.hpp>

#include "GpuTransposePlan.stub.hpp"

namespace DDA {
  namespace {
    const bool checkTransposeAlignment = false;

    struct InputComparer {
      bool operator() (const GpuTransposeDimension& dim1, const GpuTransposeDimension& dim2) {
        return dim1.inputStride < dim2.inputStride;
      }
    };
    struct OutputComparer {
      bool operator() (const GpuTransposeDimension& dim1, const GpuTransposeDimension& dim2) {
        return dim1.outputStride < dim2.outputStride;
      }
    };
  }

  template <size_t elementSize> GpuTransposePlanUntyped<elementSize>::GpuTransposePlanUntyped (const OpenCL::StubPool& pool, /*csize_t elementSize, */const std::vector<GpuTransposeDimension>& dimensions, Core::ProfilingDataPtr prof) :
    /*elementSize_ (elementSize), */
    options (pool.options ()),
    inputSize_ (0), 
    outputSize_ (0)
  {
    pool.set (stub, prof);
    for (std::vector<GpuTransposeDimension>::const_iterator it = dimensions.begin (); it != dimensions.end (); it++)
      if (it->count == 0)
        return;

    std::vector<GpuTransposeDimension> inputD, outputD;
    for (std::vector<GpuTransposeDimension>::const_iterator it = dimensions.begin (); it != dimensions.end (); it++) {
      if (it->count > 1) {
        GpuTransposeDimension dim = *it;
        dim.inputStride *= elementSize;
        dim.outputStride *= elementSize;
        inputD.push_back (dim);
        outputD.push_back (dim);
      }
    }
    sort (inputD.begin (), inputD.end (), InputComparer ());
    sort (outputD.begin (), outputD.end (), OutputComparer ());

    {
      csize_t lastStride = elementSize;
      csize_t lastCount = 1;
      inputSize_ = elementSize;
      for (std::vector<GpuTransposeDimension>::const_iterator it = inputD.begin (); it != inputD.end (); it++) {
        ASSERT (it->inputStride % lastStride == 0);
        ASSERT (lastStride * lastCount <= it->inputStride);
        ASSERT (lastStride < it->inputStride || it == inputD.begin ()); // should never happen
        lastStride = it->inputStride;
        lastCount = it->count;
        inputSize_ += it->inputStride * (it->count - 1);
      }
      //inputSize_ = lastStride * lastCount;
    }
    {
      csize_t lastStride = elementSize;
      csize_t lastCount = 1;
      outputSize_ = elementSize;
      for (std::vector<GpuTransposeDimension>::const_iterator it = outputD.begin (); it != outputD.end (); it++) {
        ASSERT (it->outputStride % lastStride == 0);
        ASSERT (lastStride * lastCount <= it->outputStride);
        ASSERT (lastStride < it->outputStride || it == outputD.begin ()); // should never happen
        lastStride = it->outputStride;
        lastCount = it->count;
        outputSize_ += it->outputStride * (it->count - 1);
      }
      //outputSize_ = lastStride * lastCount;
    }

    dimensions_ = inputD;
    size_t pos = 0;
    for (size_t i = 0; i < inputD.size (); i++)
      if (inputD[i].outputStride == outputD[0].outputStride)
        pos = i;
    using std::swap;
    if (pos > 1)
      swap (dimensions_[1], dimensions_[pos]);

    /*
      Core::OStream::getStdout () << "TrPlan:" << std::endl;
      for (std::vector<GpuTransposeDimension>::const_iterator it = dimensions_.begin (); it != dimensions_.end (); it++)
      Core::OStream::getStdout () << " " << it->count << " " << it->inputStride << " " << it->outputStride << std::endl;
    */
  }

  template <size_t elementSize> GpuTransposePlanUntyped<elementSize>::~GpuTransposePlanUntyped () {
  }

  template <size_t elementSize> void GpuTransposePlanUntyped<elementSize>::transpose (const cl::CommandQueue& queue,
                                                                                      const cl::Buffer& input, csize_t inputOffset,
                                                                                      const cl::Buffer& output, csize_t outputOffset,
                                                                                      Core::ProfilingDataPtr prof) {
    Core::ProfileHandle _p (prof, "tr");
    if (dimensions_.size () == 0) {
      if (inputSize_ != 0)
        queue.enqueueCopyBuffer (input, output, inputOffset (), outputOffset (), elementSize);
      return;
    }

    if (dimensions_.size () == 1) {
      // not tested

      stub->transpose1<elementSize> (queue, Core::checked_cast<size_t> (stub->transpose2_globalSize<elementSize> ()),
                                     input, inputOffset, dimensions_[0].inputStride,
                                     output, outputOffset, dimensions_[0].outputStride,
                                     dimensions_[0].count);
      return;
    }

    if (dimensions_.size () == 2) {
      // loop over all except the first 2 dimensions    
      std::vector<size_t> dim (dimensions_.size () - 2);
      for (size_t i = 0; i < dim.size (); i++)
        dim[i] = 0;
      bool done = dim.size () == 0;
      do {
        csize_t inOffset = 0;
        csize_t outOffset = 0;
        for (size_t i = 0; i < dim.size (); i++) {
          inOffset += dimensions_[i + 2].inputStride * dim[i];
          outOffset += dimensions_[i + 2].outputStride * dim[i];
        }
        if (0) {
          Core::OStream::getStdout () << "(";
          for (size_t i = 0; i < dim.size (); i++)
            Core::OStream::getStdout () << dim[i] << ", ";
          Core::OStream::getStdout () << ")" << std::endl;
        }
        //ASSERT (inOffset + elementSize <= inputSize ());
        //ASSERT (outOffset + elementSize <= outputSize ());
        //Core::OStream::getStdout () << inOffset << " " << outOffset << std::endl;

        if (checkTransposeAlignment) {
          ASSERT (dimensions_[0].inputStride == 8);
          ASSERT (dimensions_[1].outputStride == 8);
          ASSERT ((inOffset + inputOffset) % (16 * 8) == 0);
          ASSERT ((outOffset + outputOffset) % (16 * 8) == 0);
        }
        if (options.enableSync ()) {
          Core::ProfileHandle _p (prof, "i");
          queue.finish ();
        }
        {
          Core::ProfileHandle _p (prof, "k");
          stub->transpose2<elementSize> (queue, cl::NDRange (Core::checked_cast<size_t> (stub->transpose2_globalSize<elementSize> ())), cl::NDRange (Core::checked_cast<size_t> (stub->transpose2_workGroupSize<elementSize> ())),
                                         input, inputOffset + inOffset, dimensions_[0].inputStride, dimensions_[1].inputStride,
                                         output, outputOffset + outOffset, dimensions_[0].outputStride, dimensions_[1].outputStride,
                                         dimensions_[0].count, dimensions_[1].count);
        }
        if (options.enableSync ()) {
          Core::ProfileHandle _p (prof, "s");
          queue.finish ();
        }


        for (size_t i = 0; i < dim.size (); i++) {
          dim[i]++;
          if (dim[i] < dimensions_[i + 2].count)
            break;
          dim[i] = 0;
          if (i == dim.size () - 1)
            done = true;
        }
      } while (!done);
      return;

    }

    if (dimensions_.size () == 3) {
      // loop over all except the first 3 dimensions    
      std::vector<size_t> dim (dimensions_.size () - 3);
      for (size_t i = 0; i < dim.size (); i++)
        dim[i] = 0;
      bool done = dim.size () == 0;
      do {
        csize_t inOffset = 0;
        csize_t outOffset = 0;
        for (size_t i = 0; i < dim.size (); i++) {
          inOffset += dimensions_[i + 3].inputStride * dim[i];
          outOffset += dimensions_[i + 3].outputStride * dim[i];
        }
        if (0) {
          Core::OStream::getStdout () << "(";
          for (size_t i = 0; i < dim.size (); i++)
            Core::OStream::getStdout () << dim[i] << ", ";
          Core::OStream::getStdout () << ")" << std::endl;
        }
        //ASSERT (inOffset + elementSize <= inputSize ());
        //ASSERT (outOffset + elementSize <= outputSize ());
        //Core::OStream::getStdout () << inOffset << " " << outOffset << std::endl;

        if (checkTransposeAlignment) {
          ASSERT (dimensions_[0].inputStride == 8);
          ASSERT (dimensions_[1].outputStride == 8);
          ASSERT ((inOffset + inputOffset) % (16 * 8) == 0);
          ASSERT ((outOffset + outputOffset) % (16 * 8) == 0);
        }
        if (options.enableSync ()) {
          Core::ProfileHandle _p (prof, "i");
          queue.finish ();
        }
        {
          Core::ProfileHandle _p (prof, "k");
          cuint64_t blockCount1 = (dimensions_[0].count + stub->transpose2_elementsPerMem<elementSize> () - 1) / stub->transpose2_elementsPerMem<elementSize> ();
          cuint64_t blockCount2 = (dimensions_[1].count + stub->transpose2_elementsPerMem<elementSize> () - 1) / stub->transpose2_elementsPerMem<elementSize> ();
          cuint64_t inc = stub->transpose2_globalSize<elementSize> () / stub->transpose2_threadsPerMem<elementSize> ();
          cuint64_t inc1 = inc % blockCount1;
          inc /= blockCount1;
          cuint64_t inc2 = inc % blockCount2;
          inc /= blockCount2;
          cuint64_t inc3 = inc;
          stub->transpose3<elementSize> (queue, cl::NDRange (Core::checked_cast<size_t> (stub->transpose2_globalSize<elementSize> ())), cl::NDRange (Core::checked_cast<size_t> (stub->transpose2_workGroupSize<elementSize> ())),
                                         input, inputOffset + inOffset, dimensions_[0].inputStride, dimensions_[1].inputStride, dimensions_[2].inputStride,
                                         output, outputOffset + outOffset, dimensions_[0].outputStride, dimensions_[1].outputStride, dimensions_[2].outputStride,
                                         dimensions_[0].count, dimensions_[1].count, dimensions_[2].count,
                                         inc1, inc2, inc3);
        }
        if (options.enableSync ()) {
          Core::ProfileHandle _p (prof, "s");
          queue.finish ();
        }


        for (size_t i = 0; i < dim.size (); i++) {
          dim[i]++;
          if (dim[i] < dimensions_[i + 3].count)
            break;
          dim[i] = 0;
          if (i == dim.size () - 1)
            done = true;
        }
      } while (!done);
      return;

    }

    // loop over all except the first 4 dimensions    
    std::vector<size_t> dim (dimensions_.size () - 4);
    for (size_t i = 0; i < dim.size (); i++)
      dim[i] = 0;
    bool done = dim.size () == 0;
    do {
      csize_t inOffset = 0;
      csize_t outOffset = 0;
      for (size_t i = 0; i < dim.size (); i++) {
        inOffset += dimensions_[i + 4].inputStride * dim[i];
        outOffset += dimensions_[i + 4].outputStride * dim[i];
      }
      if (0) {
        Core::OStream::getStdout () << "(";
        for (size_t i = 0; i < dim.size (); i++)
          Core::OStream::getStdout () << dim[i] << ", ";
        Core::OStream::getStdout () << ")" << std::endl;
      }
      //ASSERT (inOffset + elementSize <= inputSize ());
      //ASSERT (outOffset + elementSize <= outputSize ());
      //Core::OStream::getStdout () << inOffset << " " << outOffset << std::endl;

      if (checkTransposeAlignment) {
        ASSERT (dimensions_[0].inputStride == 8);
        ASSERT (dimensions_[1].outputStride == 8);
        ASSERT ((inOffset + inputOffset) % (16 * 8) == 0);
        ASSERT ((outOffset + outputOffset) % (16 * 8) == 0);
      }
      if (options.enableSync ()) {
        Core::ProfileHandle _p (prof, "i");
        queue.finish ();
      }
      {
        Core::ProfileHandle _p (prof, "k");
        stub->transpose4<elementSize> (queue, cl::NDRange (Core::checked_cast<size_t> (stub->transpose2_globalSize<elementSize> ())), cl::NDRange (Core::checked_cast<size_t> (stub->transpose2_workGroupSize<elementSize> ())),
                                       input, inputOffset + inOffset, dimensions_[0].inputStride, dimensions_[1].inputStride, dimensions_[2].inputStride, dimensions_[3].inputStride,
                                       output, outputOffset + outOffset, dimensions_[0].outputStride, dimensions_[1].outputStride, dimensions_[2].outputStride, dimensions_[3].outputStride,
                                       dimensions_[0].count, dimensions_[1].count, dimensions_[2].count, dimensions_[3].count);
      }
      if (options.enableSync ()) {
        Core::ProfileHandle _p (prof, "s");
        queue.finish ();
      }


      for (size_t i = 0; i < dim.size (); i++) {
        dim[i]++;
        if (dim[i] < dimensions_[i + 4].count)
          break;
        dim[i] = 0;
        if (i == dim.size () - 1)
          done = true;
      }
    } while (!done);
    return;
  }

  template class GpuTransposePlanUntyped<8>;
  template class GpuTransposePlanUntyped<16>;
}
