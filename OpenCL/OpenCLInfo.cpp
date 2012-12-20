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

// Show information about available opencl devices

#include <Core/Util.hpp>
#include <Core/Assert.hpp>
#include <Core/OStream.hpp>

#include <OpenCL/Bindings.hpp>
#include <OpenCL/GetError.hpp>

#include <sstream>
#include <set>

#include <cmath>
#include <cstdlib>

#include <CL/cl_ext.h>

template <cl_uint info, class T> std::set<std::string> getExtensions (const T& object) {
  std::set<std::string> set;
  std::string extensions = object.template getInfo<info> () + " ";
  size_t pos = 0;
  size_t pos_old = 0;
  while ((pos = extensions.find (" ", pos)) != std::string::npos) {
    if (extensions.substr (pos_old, pos - pos_old).find_first_not_of (' ') != std::string::npos)
      set.insert (extensions.substr (pos_old, pos - pos_old));
    pos = pos + 1;
    pos_old = pos;
  }
  return set;
}

//#define CL_PLATFORM_ICD_SUFFIX_KHR          0x0920

template <typename Ty, typename T> std::string get (const T& object, cl_uint info, const char* infoName = 0) {
  std::stringstream str;
  try {
    Ty result;
    object.getInfo (info, &result);
    str << result;
  } catch (OpenCL::Error& e) {
    str.setf (std::ios::hex, std::ios::basefield);
    str << "<" << e.errStr () << "(";
    if (infoName)
      str << infoName;
    else
      str << "0x" << info;
    str << ") returned " << OpenCL::getErrorString (e.err ()) << ">";
  }
  return str.str ();
}

template <typename T>
inline std::ostream& operator<< (std::ostream& stream, const std::vector<T>& v) {
  stream << "(";
  bool first = true;
  for (typename std::vector<T>::const_iterator it = v.begin (); it != v.end (); it++) {
    if (!first)
      stream << ", ";
    first = false;
    stream << *it;
  }
  stream << ")";
  return stream;
}

template <cl_uint info, typename T> std::string get_nt (const T& object, const char* infoName = 0) {
  std::stringstream str;
  try {
    str << object.template getInfo<info> ();
  } catch (OpenCL::Error& e) {
    str.setf (std::ios::hex, std::ios::basefield);
    str << "<" << e.errStr () << "(";
    if (infoName)
      str << infoName;
    else
      str << "0x" << info;
    str << ") returned " << OpenCL::getErrorString (e.err ()) << ">";
  }
  return str.str ();
}

#define GET(o, ty, info) get<ty> (o, info, #info)

int main () {
  //cl::Context context (CL_DEVICE_TYPE_CPU);
  //std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES> ();

  std::vector<cl::Platform> platforms;
  cl::Platform::get (&platforms);
  //platforms.push_back (cl::Platform ());

#define INFO_T(ty, info) Core::OStream::getStdout () << ident << #info " = " << get<ty> (obj, info, #info) << std::endl
#define INFO(info) Core::OStream::getStdout () << ident << #info " = " << get_nt<info> (obj, #info) << std::endl

  for (std::vector<cl::Platform>::iterator pit = platforms.begin (); pit != platforms.end (); pit++) {
    std::set<std::string> plExt = getExtensions<CL_PLATFORM_EXTENSIONS> (*pit);
    __typeof__ (*pit)& obj = *pit;
    std::string ident = "";

    INFO (CL_PLATFORM_PROFILE);
    INFO (CL_PLATFORM_VERSION);
    INFO (CL_PLATFORM_NAME);
    INFO (CL_PLATFORM_VENDOR);
    INFO (CL_PLATFORM_EXTENSIONS);
    if (plExt.find ("cl_khr_icd") != plExt.end ()) {
      INFO_T (std::string, CL_PLATFORM_ICD_SUFFIX_KHR);
    } else {
      Core::OStream::getStdout () << ident << "No cl_khr_icd available" << std::endl;
    }
    //Core::OStream::getStdout () << pit->getInfo<CL_PLATFORM_PROFILE> () << " / " << pit->getInfo<CL_PLATFORM_VERSION> () << " / " << pit->getInfo<CL_PLATFORM_NAME> () << " / " << pit->getInfo<CL_PLATFORM_VENDOR> () << " / " << pit->getInfo<CL_PLATFORM_EXTENSIONS> () << " / " << GET (*pit, std::string, CL_PLATFORM_ICD_SUFFIX_KHR) << std::endl;

    std::vector<cl::Device> devices;
    pit->getDevices (CL_DEVICE_TYPE_ALL, &devices);

    for (std::vector<cl::Device>::iterator it = devices.begin (); it != devices.end (); it++) {
      __typeof__ (*it)& obj = *it;
      std::string ident = " ";
      INFO (CL_DEVICE_TYPE);
      INFO (CL_DEVICE_NAME);
      ident = "    ";
      /*
      INFO (CL_DEVICE_VENDOR);
      INFO (CL_DEVICE_VENDOR_ID);
      INFO (CL_DRIVER_VERSION);
      INFO (CL_DEVICE_PROFILE);
      INFO (CL_DEVICE_VERSION);
      INFO (CL_DEVICE_EXTENSIONS);
      INFO (CL_DEVICE_PROFILING_TIMER_RESOLUTION);
      INFO (CL_DEVICE_MAX_COMPUTE_UNITS);
      INFO (CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
      INFO (CL_DEVICE_MAX_WORK_ITEM_SIZES);
      */
      //INFO (CL_DEVICE_TYPE);
      INFO (CL_DEVICE_VENDOR_ID);
      INFO (CL_DEVICE_MAX_COMPUTE_UNITS);
      INFO (CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
      INFO (CL_DEVICE_MAX_WORK_GROUP_SIZE);
      INFO (CL_DEVICE_MAX_WORK_ITEM_SIZES);
      INFO (CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR);
      INFO (CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT);
      INFO (CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT);
      INFO (CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG);
      INFO (CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT);
      INFO (CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE);
      INFO (CL_DEVICE_MAX_CLOCK_FREQUENCY);
      INFO (CL_DEVICE_ADDRESS_BITS);
      INFO (CL_DEVICE_MAX_READ_IMAGE_ARGS);
      INFO (CL_DEVICE_MAX_WRITE_IMAGE_ARGS);
      INFO (CL_DEVICE_MAX_MEM_ALLOC_SIZE);
      INFO (CL_DEVICE_IMAGE2D_MAX_WIDTH);
      INFO (CL_DEVICE_IMAGE2D_MAX_HEIGHT);
      INFO (CL_DEVICE_IMAGE3D_MAX_WIDTH);
      INFO (CL_DEVICE_IMAGE3D_MAX_HEIGHT);
      INFO (CL_DEVICE_IMAGE3D_MAX_DEPTH);
      INFO (CL_DEVICE_IMAGE_SUPPORT);
      INFO (CL_DEVICE_MAX_PARAMETER_SIZE);
      INFO (CL_DEVICE_MAX_SAMPLERS);
      INFO (CL_DEVICE_MEM_BASE_ADDR_ALIGN);
      INFO (CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE);
      INFO (CL_DEVICE_SINGLE_FP_CONFIG);
      INFO (CL_DEVICE_GLOBAL_MEM_CACHE_TYPE);
      INFO (CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE);
      INFO (CL_DEVICE_GLOBAL_MEM_CACHE_SIZE);
      INFO (CL_DEVICE_GLOBAL_MEM_SIZE);
      INFO (CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE);
      INFO (CL_DEVICE_MAX_CONSTANT_ARGS);
      INFO (CL_DEVICE_LOCAL_MEM_TYPE);
      INFO (CL_DEVICE_LOCAL_MEM_SIZE);
      INFO (CL_DEVICE_ERROR_CORRECTION_SUPPORT);
      INFO (CL_DEVICE_PROFILING_TIMER_RESOLUTION);
      INFO (CL_DEVICE_ENDIAN_LITTLE);
      INFO (CL_DEVICE_AVAILABLE);
      INFO (CL_DEVICE_COMPILER_AVAILABLE);
      INFO (CL_DEVICE_EXECUTION_CAPABILITIES);
      INFO (CL_DEVICE_QUEUE_PROPERTIES);
      //INFO (CL_DEVICE_NAME);
      INFO (CL_DEVICE_VENDOR);
      INFO (CL_DRIVER_VERSION);
      INFO (CL_DEVICE_PROFILE);
      INFO (CL_DEVICE_VERSION);
      INFO (CL_DEVICE_EXTENSIONS);
      //INFO (CL_DEVICE_PLATFORM);
      if (plExt.find ("cl_nv_device_attribute_query") != plExt.end ()) {
        INFO_T (cl_uint, CL_DEVICE_COMPUTE_CAPABILITY_MAJOR_NV);
        INFO_T (cl_uint, CL_DEVICE_COMPUTE_CAPABILITY_MINOR_NV);
        INFO_T (cl_uint, CL_DEVICE_REGISTERS_PER_BLOCK_NV);
        INFO_T (cl_uint, CL_DEVICE_WARP_SIZE_NV);
        INFO_T (cl_bool, CL_DEVICE_GPU_OVERLAP_NV);
        INFO_T (cl_bool, CL_DEVICE_KERNEL_EXEC_TIMEOUT_NV);
        INFO_T (cl_bool, CL_DEVICE_INTEGRATED_MEMORY_NV);
      } else {
        Core::OStream::getStdout () << ident << "No cl_nv_device_attribute_query available" << std::endl;
      }
    }
  }
}
