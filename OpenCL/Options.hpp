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

#ifndef OPENCL_OPTIONS_HPP_INCLUDED
#define OPENCL_OPTIONS_HPP_INCLUDED

// Class containing options for opencl execution (currently just whether
// CommandQueue::finish() should be called after every step)

#include <OpenCL/Bindings.hpp>

#include <boost/shared_ptr.hpp>

namespace OpenCL {
  class Options {
    struct Shared {
      bool enableSync;

      Shared ()
        : enableSync (false)
      {
      }
    };

    boost::shared_ptr<Shared> shared;

    Options () : shared (new Shared ()) {}

  public:
    static Options create () {
      return Options ();
    }

    bool enableSync () const { return shared->enableSync; }
    void enableSync (bool enableSync) const {
      shared->enableSync = enableSync;
    }

    void sync (const cl::CommandQueue& queue) const {
      if (enableSync ())
        queue.finish ();
    }
  };
}

#endif // !OPENCL_OPTIONS_HPP_INCLUDED
