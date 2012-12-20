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

#include "Bindings.hpp"

#include <Core/Exception.hpp>
#include <Core/Assert.hpp>
#include <Core/StringUtil.hpp>
#include <Core/OStream.hpp>

#include <OpenCL/GetError.hpp>

#include <sstream>

#include <boost/algorithm/string.hpp>

namespace OpenCL {
  Error::Error (cl_int err, const char * errStr) : err_(err), errStr_(errStr) {}
  Error::~Error () throw() {}

  std::string Error::message () const {
    std::stringstream str;
    str << "OpenCL Error: " << (errStr_ ? errStr_ : "empty") << ": " << getErrorString (err ());
    return str.str ();
  }

  static std::string logsToString (const std::vector<std::string>& logs) {
    std::stringstream str;
    for (size_t i = 0; i < logs.size (); i++) {
      str << "Build log for device " << i << ":";
      std::string log = logs[i];
      log = "\n" + log;
      if (log.length () > 0 && log[log.length () - 1] == '\n')
        log = log.substr (0, log.length () - 1);
      str << Core::findReplace (log, "\n", "\n> ") << std::endl << std::endl;
    }
    return str.str ();
  }

  BuildError::BuildError (cl_int err, const char* errStr, const std::vector<std::string>& logs) :
    Error (err, errStr), logs_ (logs) {
  }

  BuildError::~BuildError () throw () {}

  std::string BuildError::message () const {
    std::stringstream str;
    str << "OpenCL Build Error: " << (errStr () ? errStr () : "empty") << ": " << getErrorString (err ());
    str << std::endl << logsToString (logs ());
    return str.str ();
  }

  void clErrorHandler (cl_int err, const char * errStr) {
    throw Error(err, errStr);
  }

  std::vector<std::string> callClProgramBuild (const cl::Program& program, const std::vector<cl::Device>& devices, const std::string& options) {
    cl_int ret = clBuildProgram (program (), (cl_uint) devices.size (), (const cl_device_id*) devices.data (), options.c_str (), NULL, NULL);

    std::vector<std::string> logs;
    for (std::vector<cl::Device>::const_iterator it = devices.begin (); it != devices.end (); it++)
      logs.push_back (program.getBuildInfo<CL_PROGRAM_BUILD_LOG> (*it));
  
    if (ret != CL_SUCCESS)
      throw BuildError (ret, "clBuildProgram", logs);

    return logs;
  }

  void callClProgramBuildAndShowWarnings (const cl::Program& program, const std::vector<cl::Device>& devices, const std::string& options) {
    std::vector<std::string> logs = callClProgramBuild (program, devices, options);
    bool foundWarning = false;
    for (size_t i = 0; i < logs.size () && !foundWarning; i++)
      if (boost::trim_copy (logs[i]) != "")
        foundWarning = true;
    if (foundWarning)
      Core::OStream::getStderr () << "Got warnings while compiling OpenCL code:" << std::endl << logsToString (logs) << std::flush;
  }

  size_t getDefaultWorkItemCount (const cl::Device& device) {
    return device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS> () * 512;
  }
  size_t getDefaultWorkItemCount (const cl::CommandQueue& queue) {
    return getDefaultWorkItemCount (queue.getInfo<CL_QUEUE_DEVICE> ());
  }

  // Workaround for refcount problem with program.getInfo<CL_PROGRAM_CONTEXT> ();
  cl::Context clProgramGetContext (const cl::Program& program) {
    cl::Context context;
    cl_context c;
    cl::detail::errHandler (clGetProgramInfo (program (), CL_PROGRAM_CONTEXT, sizeof (c), &c, NULL), "clGetProgramInfo");
    cl::detail::errHandler (clRetainContext (c), "clRetainContext");
    context () = c;
    return context;
  }
}
