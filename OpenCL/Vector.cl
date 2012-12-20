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

#include <OpenCL/Common.h>
#include <OpenCL/Float.h>

#define DEF_OPS(T)                                                      \
  __kernel void multiplyScalar__##T (__global T* out, __global T* in, T scalar, uint count) { \
    for (uint i = get_global_id (0); i < count; i += get_global_size (0)) \
      out[i] = T##_mul (in[i], scalar);                                 \
  }                                                                     \
                                                                        \
  __kernel void multiplyElementwise__##T (__global T* out, __global T* in1, __global T* in2, uint count) { \
    for (uint i = get_global_id (0); i < count; i += get_global_size (0)) \
      out[i] = T##_mul (in1[i], in2[i]);                                \
  }                                                                     \
                                                                        \
  __kernel void multiplyElementwiseScalar__##T (__global T* out, __global T* in1, __global T* in2, T scalar, uint count) { \
    for (uint i = get_global_id (0); i < count; i += get_global_size (0)) \
      out[i] = T##_mul (T##_mul (in1[i], in2[i]), scalar);              \
  }

DEF_OPS(float) DEF_OPS(cfloat)
#if CL_HAVE_DOUBLE
DEF_OPS(double) DEF_OPS(cdouble)
#endif

// Local Variables: 
// mode: c
// End: 
