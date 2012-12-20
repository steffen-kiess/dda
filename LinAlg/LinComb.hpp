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

#ifndef LINALG_LINCOMB_HPP_INCLUDED
#define LINALG_LINCOMB_HPP_INCLUDED

// Code for doing linear combinations and vector reductions on the CPU

#include <vector>

#include <cstddef>

namespace LinAlg {
  template <typename F, typename S> void linComb (const std::vector<F>& v1, S s1, std::vector<F>& out) {
    ASSERT (v1.size() == out.size ());
    for (size_t i = 0; i < out.size (); i++)
      out[i] = v1[i] * s1;
  }

  template <typename F, typename S> void linComb (const std::vector<F>& v1, S s1, const std::vector<F>& v2, std::vector<F>& out) {
    ASSERT (v1.size() == out.size ());
    ASSERT (v2.size() == out.size ());
    for (size_t i = 0; i < out.size (); i++)
      out[i] = v1[i] * s1 + v2[i];
  }
  template <typename F, typename S> void linComb (const std::vector<F>& v1, S s1, const std::vector<F>& v2, S s2, std::vector<F>& out) {
    ASSERT (v1.size() == out.size ());
    ASSERT (v2.size() == out.size ());
    for (size_t i = 0; i < out.size (); i++)
      out[i] = v1[i] * s1 + v2[i] * s2;
  }

  template <typename F, typename S> void linComb (const std::vector<F>& v1, S s1, const std::vector<F>& v2, S s2, const std::vector<F>& v3, std::vector<F>& out) {
    ASSERT (v1.size() == out.size ());
    ASSERT (v2.size() == out.size ());
    ASSERT (v3.size() == out.size ());
    for (size_t i = 0; i < out.size (); i++)
      out[i] = v1[i] * s1 + v2[i] * s2 + v3[i];
  }
  template <typename F, typename S> void linComb (const std::vector<F>& v1, S s1, const std::vector<F>& v2, S s2, const std::vector<F>& v3, S s3, std::vector<F>& out) {
    ASSERT (v1.size() == out.size ());
    ASSERT (v2.size() == out.size ());
    ASSERT (v3.size() == out.size ());
    for (size_t i = 0; i < out.size (); i++)
      out[i] = v1[i] * s1 + v2[i] * s2 + v3[i] * s3;
  }

  template <typename F> __decltype(norm (*(F*)0)) norm (const std::vector<F>& v) {
    typedef __decltype(norm (*(F*)0)) R;
    R result = 0;
    for (size_t i = 0; i < v.size (); i++)
      result += norm (v[i]);
    return result;
  }

  template <typename T> static void fill (std::vector<T>& array, const T& value) {
    std::fill (array.begin (), array.end (), value);
  }
}

#endif // !LINALG_LINCOMB_HPP_INCLUDED
