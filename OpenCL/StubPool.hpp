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

#ifndef OPENCL_STUBPOOL_HPP_INCLUDED
#define OPENCL_STUBPOOL_HPP_INCLUDED

// A OpenCL::StubPool contains a list of stub types instances.
// When a stub type not in the list is requested a new instance is created.

#include <Core/Assert.hpp>
#include <Core/Profiling.hpp>

#include <OpenCL/Bindings.hpp>
#include <OpenCL/Options.hpp>

#include <map>
#include <typeinfo>

#include <boost/any.hpp>
#include <boost/static_assert.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

namespace OpenCL {
  class StubPool {
    struct TypeinfoCompare {
      bool operator() (const std::type_info* i1, const std::type_info* i2) {
        return i1->before (*i2);
      }
    };
    struct Shared {
      cl::Context context;
      boost::mutex mutex;
      std::map<const std::type_info*, boost::any, TypeinfoCompare> data;
      Options options;

      Shared (const cl::Context& context, const Options& options) : context (context), options (options) {
      }
    };
    boost::shared_ptr<Shared> shared;

  public:
    explicit StubPool (const cl::Context& context, const Options& options = Options::create ()) : shared (new Shared (context, options)) {}

    template <typename T> boost::shared_ptr<T> get (Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      boost::lock_guard<boost::mutex> guard (shared->mutex);
      std::map<const std::type_info*, boost::any, TypeinfoCompare>::iterator it = shared->data.find (&typeid (T));
      if (it != shared->data.end ())
        return boost::any_cast<boost::shared_ptr<T>&> (it->second);

      //Core::OStream::getStdout () << typeid (T).name () << " " << &*prof << std::endl;
      Core::ProfileHandle _p (prof, std::string ("init ") + typeid (T).name ());

      boost::shared_ptr<T> value = T::create (shared->context);
      shared->data[&typeid (T)] = boost::any (value);

      it = shared->data.find (&typeid (T));
      ASSERT (it != shared->data.end ());
      return boost::any_cast<boost::shared_ptr<T>&> (it->second);
    }

    template <typename T> void set (boost::shared_ptr<T>& ptr, Core::ProfilingDataPtr prof = Core::ProfilingDataPtr ()) const {
      ptr = get<T> (prof);
    }

    const cl::Context& context () const {
      return shared->context;
    }

    const Options& options () const {
      return shared->options;
    }
  };
}

#endif // !OPENCL_STUBPOOL_HPP_INCLUDED
