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

#ifndef OPENCL_BINDINGS_HPP_INCLUDED
#define OPENCL_BINDINGS_HPP_INCLUDED

// Include CL/cl.hpp and use a custom error handler (which throws an exception)

#include <Core/Util.hpp>
#include <Core/Exception.hpp>

#include <CL/cl.h>

namespace OpenCL {
  class Error : public Core::Exception {
  private:
    cl_int err_;
    const char * errStr_;

  public:
    Error (cl_int err, const char * errStr = NULL);
    ~Error () throw ();

    cl_int err () const { return err_; }
    const char* errStr () const { return errStr_; }

    virtual std::string message () const;
  };

  class BuildError : public Error {
  private:
    std::vector<std::string> logs_;

  public:
    BuildError (cl_int err, const char* errStr, const std::vector<std::string>& logs);
    ~BuildError () throw ();

    const std::vector<std::string>& logs () const { return logs_; }

    virtual std::string message () const;
  };


  NORETURN clErrorHandler (cl_int err, const char * errStr);

  static inline void clErrorHandler2 (cl_int err, const char * errStr) {
    if (err != CL_SUCCESS) {
      clErrorHandler (err, errStr);
    }
  }
}

#if (OS_WIN || defined (__CYGWIN__)) && defined (__GNUC__) && !defined (alloca)
#define alloca __builtin_alloca
#define USE_OWN_alloca
#endif

#define __CL_CUSTOM_ERROR_HANDLER OpenCL::clErrorHandler2
#include <CL/cl.hpp>

#ifdef USE_OWN_alloca
#undef alloca
#undef USE_OWN_alloca
#endif

namespace OpenCL {
  std::vector<std::string> callClProgramBuild (const cl::Program& program, const std::vector<cl::Device>& devices, const std::string& options = "");
  void callClProgramBuildAndShowWarnings (const cl::Program& program, const std::vector<cl::Device>& devices, const std::string& options = "");

  size_t getDefaultWorkItemCount (const cl::Device& device);
  size_t getDefaultWorkItemCount (const cl::CommandQueue& queue);

  // Workaround refcount problem with program.getInfo<CL_PROGRAM_CONTEXT> ();
  cl::Context clProgramGetContext (const cl::Program& program);
}

#endif // !OPENCL_BINDINGS_HPP_INCLUDED
