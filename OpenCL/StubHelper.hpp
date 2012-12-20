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

#ifndef OPENCL_STUBHELPER_HPP_INCLUDED
#define OPENCL_STUBHELPER_HPP_INCLUDED

// Some utility functions used by the .stub.hpp files

#include <Core/Util.hpp>
#include <Core/StringUtil.hpp>
#include <Core/Assert.hpp>
#include <Core/CheckedInteger.hpp>

#include <OpenCL/Bindings.hpp>
#include <OpenCL/Vector.hpp>

#include <map>
#include <set>
#include <string>
#include <algorithm>
#include <typeinfo>
#include <stack>
#include <iterator>
#include <sstream>

#include <cstddef>

#include <stdint.h>

#include <CL/cl_ext.h>

#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/exceptions.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

namespace OpenCL {
  namespace StubHelper {
    // Wrapper for a pointer argument, value can be a cl::Buffer, a cl_mem or
    // a OpenCL::Vector<>
    template <typename T> class BufferWr {
      cl_mem value_;

    public:
      BufferWr (const cl::Buffer& buffer) : value_ (buffer ()) {}
      BufferWr (cl_mem buffer) : value_ (buffer) {}
      BufferWr (OpenCL::Vector<T>& vector) : value_ (vector.getDataWritable () ()) {}

      const cl_mem& operator() () const {
        return value_;
      }

      cl_mem& value () {
        return value_;
      }
    };

    // Wrapper for a const pointer argument
    template <typename T> class ConstBufferWr {
      cl_mem value_;

    public:
      ConstBufferWr (const cl::Buffer& buffer) : value_ (buffer ()) {}
      ConstBufferWr (cl_mem buffer) : value_ (buffer) {}
      ConstBufferWr (const OpenCL::Vector<T>& vector) : value_ (vector.getData () ()) {}

      const cl_mem& operator() () const {
        return value_;
      }

      cl_mem& value () {
        return value_;
      }
    };

    // Wrapper for an integer value, does range checks
    template <typename T> class IntWr {
      T value_;

    public:
      template <typename U> IntWr (U value) : value_ (Core::checked_cast<T> (value)) {}

      T& value () {
        return value_;
      }
    };

    // A list of kernels used to allow multiple threads to call the same
    // kernel at the same time (calling a kernel is not thread-safe because
    // setting the kernel arguments modifies the kernel object).
    class KernelContainer {
      boost::mutex mutex;
      std::stack<cl::Kernel> kernels;

    public:
      class Handle {
        KernelContainer& container;
        cl::Kernel kernel;

      public:
        Handle (KernelContainer& container, const cl::Program& program, const char* name);
        ~Handle ();

        const cl::Kernel& operator() () {
          return kernel;
        }
      };
    };

    struct GetEvent_t {};
    struct NoEvent_t {};

    std::set<std::string> getDeviceExtensions (const cl::Device& device);
    bool contextHasDoubleSupport (const cl::Context& context);
  }
  static const StubHelper::GetEvent_t getEvent = StubHelper::GetEvent_t ();
  static const StubHelper::NoEvent_t noEvent = StubHelper::NoEvent_t ();

  // Base class for all stub classes
  template <typename T> class StubBase {
    typedef std::pair<const char*, size_t> SourceDataType;
    typedef std::pair<const void*, size_t> DataType;

    static SourceDataType getSourceIntern () {
      return std::make_pair (T::dataSource, T::lenSource);
    }
    
  public:
    static const SourceDataType& getSource () {
      static const SourceDataType data = getSourceIntern ();
      return data;
    }
    static const std::string& getCompileOptions () {
      static const std::string data = T::compileOptions;
      return data;
    }

    static cl::Program getProgram (const cl::Context& context) {
      std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES> ();
      cl::Program program;
      std::string options = getCompileOptions ();
      program = cl::Program (context, cl::Program::Sources (1, getSource ()));
      //Core::OStream::getStderr () << "Options '" << options << "'" << std::endl;
      callClProgramBuildAndShowWarnings (program, devices, options);
      return program;
    }
  };
}

#endif // !OPENCL_STUBHELPER_HPP_INCLUDED
