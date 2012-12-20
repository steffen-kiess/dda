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

#ifndef DDA_GPUFFTPLANS_HPP_INCLUDED
#define DDA_GPUFFTPLANS_HPP_INCLUDED

// Method for getting a GPU FFT plan for certian parameters

#include <OpenCL/StubPool.hpp>

#include <LinAlg/GpuFFTPlan.hpp>

#include <boost/program_options.hpp>

namespace DDA {
  template <class ftype> const LinAlg::GpuFFTPlanFactory<ftype>& getPlanFactory (const boost::program_options::variables_map& map, UNUSED const OpenCL::StubPool& pool);
}

#endif // !DDA_GPUFFTPLANS_HPP_INCLUDED
