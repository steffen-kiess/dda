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

#ifndef CORE_UTIL_HPP_INCLUDED
#define CORE_UTIL_HPP_INCLUDED

// Various preprocessor macros for C++

#include <Core/Util.h>

// HAVE_CXX11 is true iff there is (at least some) support for C++11
#if defined(__GXX_EXPERIMENTAL_CXX0X__) || __cplusplus >= 201103L
#define HAVE_CXX11 1
#else
#define HAVE_CXX11 0
#endif

#if !HAVE_CXX11

// Can be used to disable the copy constructor and assignment operator of a
// class
#define NO_COPY_CLASS(n)                                        \
  private:                                                      \
  ERROR_ATTRIBUTE ("Class " #n " has no assignment operator")   \
  n& operator= (const n &x);                                    \
  ERROR_ATTRIBUTE ("Class " #n " has no copy constructor")      \
  n (const n &x)

// Can be used to disable the default constructor and destructor of a class
#define STATIC_CLASS(n)                                 \
  NO_COPY_CLASS (n);                                    \
private:                                                \
 ERROR_ATTRIBUTE ("Class " #n " cannot be constructed") \
 n ();                                                  \
 ERROR_ATTRIBUTE ("Class " #n " cannot be constructed") \
 ~n ()

#else // HAVE_CXX11

// Versions for C++11

#define NO_COPY_CLASS(n)                        \
  private:                                      \
  n& operator= (const n &x) = delete;           \
  n (const n &x) = delete

#define STATIC_CLASS(n)                         \
  NO_COPY_CLASS (n);                            \
private:                                        \
 n () = delete;                                 \
 ~n () = delete

#endif // HAVE_CXX11

#ifdef __CUDACC__
#define NVCC_HOST_DEVICE __host__ __device__
#else
#define NVCC_HOST_DEVICE
#endif

#if defined (__CUDACC__) && defined (CUDART_VERSION) && CUDART_VERSION < 5000
#define DECLTYPE(t) typeof (t)
#else
#define DECLTYPE(t) __decltype (t)
#endif

#endif // !CORE_UTIL_HPP_INCLUDED
