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

#ifndef DDA_DEBUG_HPP_INCLUDED
#define DDA_DEBUG_HPP_INCLUDED

// Debugging aids which will print the content of a vector to stdout

#include <Core/OStream.hpp>

#include <OpenCL/Bindings.hpp>

#include <DDA/DipVector.hpp>

#include <vector>

namespace DDA {
  namespace Debug {
    template <typename F> __decltype(norm (*(F*)0)) norm (const std::vector<F>& v) {
      typedef __decltype(norm (*(F*)0)) R;
      R result = 0;
      for (size_t i = 0; i < v.size (); i++)
        result += norm (v[i]);
      return result;
    }
    template <typename F> inline void info (const char* s, const std::vector<cl::CommandQueue>& queues, const DipVector<F>& v) {
      boost::shared_ptr<std::vector<std::complex<F> > > i = v.read (queues);
      Core::OStream::getStdout () << s << " = [" << norm<F> (i) << ", " << vecProdConj (queues, v, v) << "] (" << i[0] << ", " << i[1] << ", ..., " << i[i.size () - 2] << ", " << i[i.size () - 1] << ")" << i[3754] << " " << i[3884] << std::endl;
      F rmin = 1000, imin = 1000, rmax = -1000, imax = -1000;
      size_t a = 0;
      for (size_t j = 0; j < i.size (); j++) {
        if (i[j].real () < rmin) {
          rmin = i[j].real ();
          a = j;
        }
        if (i[j].real () > rmax)
          rmax = i[j].real ();
        if (i[j].imag () < imin)
          imin = i[j].imag ();
        if (i[j].imag () > imax)
          imax = i[j].imag ();
      }
      //Core::OStream::getStdout () << a << " " << rmin << " " << imin << " " << rmax << " " << imax << std::endl;
    }
    template <typename F> inline void info (const char* s, UNUSED const std::vector<cl::CommandQueue>& queues, std::complex<F> v) {
      Core::OStream::getStdout () << s << " = " << v << std::endl;
    }
    template <typename F> inline void info (const char* s, UNUSED const std::vector<cl::CommandQueue>& queues, F v) {
      Core::OStream::getStdout () << s << " = " << v << std::endl;
    }
    template <typename F> inline void info (const char* s, const std::vector<cl::CommandQueue>& queues, const OpenCL::Pointer<const F>& p) {
      F v = p.read (queues[0]);
      Core::OStream::getStdout () << s << " = " << v << std::endl;
    }
    template <typename F> inline void info (const char* s, const std::vector<cl::CommandQueue>& queues, const OpenCL::Pointer<F>& p) {
      F v = p.read (queues[0]);
      Core::OStream::getStdout () << s << " = " << v << std::endl;
    }
    inline void info (UNUSED const char* s, UNUSED const std::vector<cl::CommandQueue>& queues, const char* st) {
      Core::OStream::getStdout () << st << std::endl;
    }
  }
}

#endif // !DDA_DEBUG_HPP_INCLUDED
