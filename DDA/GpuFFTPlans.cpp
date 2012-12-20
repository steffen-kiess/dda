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

#include "GpuFFTPlans.hpp"

#include <LinAlg/GpuFFTPlanCl.hpp>

#include <boost/foreach.hpp>

#ifdef ADDITIONAL_OPENCL_FFT_FACTORIES_DECLS
  ADDITIONAL_OPENCL_FFT_FACTORIES_DECLS
#endif

namespace DDA {
  template <class ftype> const LinAlg::GpuFFTPlanFactory<ftype>& getPlanFactory (const boost::program_options::variables_map& map, UNUSED const OpenCL::StubPool& pool) {
    std::map<std::string, const LinAlg::GpuFFTPlanFactory<ftype>* > factories;
    factories["cl"] = &LinAlg::getGpuFFTPlanClFactory<ftype> ();
#ifdef ADDITIONAL_OPENCL_FFT_FACTORIES
    ADDITIONAL_OPENCL_FFT_FACTORIES
#endif

      std::string name;
    if (map.count ("opencl-fft")) {
      name = map["opencl-fft"].as<std::string> ();
    } else {
      name = "cl";
    }

    typename std::map<std::string, const LinAlg::GpuFFTPlanFactory<ftype>* >::const_iterator ir = factories.find (name);
    if (ir == factories.end ()) {
      std::string valid;
      typedef std::pair<std::string, const LinAlg::GpuFFTPlanFactory<ftype>* > PairType;
      BOOST_FOREACH (PairType pair, factories) {
        if (valid != "")
          valid += ", ";
        valid += pair.first;
      }
      ABORT_MSG ("Unknown OpenCL fft implementation `" + name + "', known ones are " + valid);
    } else {
      return *ir->second;
    }
  }

#define TEMPL(ftype, IGNORE) template const LinAlg::GpuFFTPlanFactory<ftype>& getPlanFactory (const boost::program_options::variables_map& map, UNUSED const OpenCL::StubPool& pool);
  CALL_MACRO_FOR_OPENCL_FP_TYPES (TEMPL, IGNORE)
#undef TEMPL
}
