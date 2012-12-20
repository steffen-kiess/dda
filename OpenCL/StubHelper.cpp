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

#include "StubHelper.hpp"

namespace OpenCL {
  namespace StubHelper {
    KernelContainer::Handle::Handle (KernelContainer& container, const cl::Program& program, const char* name) : container (container) {
      {
        boost::lock_guard<boost::mutex> guard (container.mutex);
        if (!container.kernels.empty ()) {
          kernel = container.kernels.top ();
          container.kernels.pop ();
          return;
        }
      }
      kernel = cl::Kernel (program, name);
    }

    KernelContainer::Handle::~Handle () {
      boost::lock_guard<boost::mutex> guard (container.mutex);
      container.kernels.push (kernel);
    }

    std::set<std::string> getDeviceExtensions (const cl::Device& device) {
      std::set<std::string> set;
      BOOST_FOREACH (const std::string& s, Core::split (device.getInfo<CL_DEVICE_EXTENSIONS> (), " ")) {
        if (s != "")
          set.insert (s);
      }
      return set;
    }

    bool contextHasDoubleSupport (const cl::Context& context) {
      std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES> ();
      for (std::vector<cl::Device>::const_iterator it = devices.begin (); it != devices.end (); it++) {
        std::set<std::string> extensions = getDeviceExtensions (*it);
        if (extensions.find ("cl_khr_fp64") == extensions.end () && extensions.find ("cl_amd_fp64") == extensions.end ())
          return false;
      }
      return true;
    }
  }
}
