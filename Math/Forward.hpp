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

#ifndef MATH_FORWARD_HPP_INCLUDED
#define MATH_FORWARD_HPP_INCLUDED

// Forward declarations of types in Math namespace

#include <cstddef>

#include <stdint.h>

namespace Math {
  template <typename T> class Vector3;
  template <typename T> class DiagMatrix3;
  template <typename T> class SymMatrix3;

  template <bool enabled> struct ArrayAssertions;
  struct ArrayConfig;
  template <typename Config, typename T> class ArrayAllocator;
  template <typename Config, typename T> class DefaultArrayAllocator;
  template <std::size_t dim, bool nonzeroLB> class ArrayViewBoundsProvider;
  template <std::size_t dim, bool nonzeroLB, typename Config> class ArrayViewBase;
  template <typename T, std::size_t dim = 1, bool nonzeroLB = false, typename Config = ArrayConfig, typename Assert = typename Config::DefaultAssert> class ArrayView;
  template <typename T, std::size_t dim = 1, typename Config = ArrayConfig, typename Assert = typename Config::DefaultAssert> class Array;
  //template <typename T, std::size_t dim, typename Config, typename Assert> class ArrayView;
  //template <typename T, std::size_t dim, typename Config, typename Assert> class Array;

  // Must be here because of default template parameter for ArrayView
  struct ArrayConfig {
    typedef ArrayAssertions<true> DefaultAssert;

    // Use uintptr_t instead of char* because of non-zero lower bounds
    //typedef char* ArithmeticPointer;
    typedef uintptr_t ArithmeticPointer;
    static inline ArithmeticPointer getNull () { return 0; }

    template <typename T> struct Type;
  };
}

#endif // !MATH_FORWARD_HPP_INCLUDED
