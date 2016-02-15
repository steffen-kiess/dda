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

#ifndef OPENCL_COMMON_H_INCLUDED
#define OPENCL_COMMON_H_INCLUDED

// Declarations which are used both by host code and by opencl device code

#ifdef __OPENCL_VERSION__

// Device code

#define S(x) typedef x cl_##x
#define D(x) S(x); S(x##2); S(x##4); S(x##8); S(x##16)
D(char); D(uchar);
D(short); D(ushort);
D(int); D(uint);
D(long); D(ulong);
//S(half);
D(float); //D(double);
#undef D
#undef S

#define OPENCL_PTR(x) x __global*

#else

// Host code

#include <stdint.h>

#define OPENCL_PTR(x) uint32_t

#endif

#define CL_CONCAT_IMPL(a, b) a##b
#define CL_CONCAT(a, b) CL_CONCAT_IMPL(a, b)
#define CL_CONCAT3(a, b, c) CL_CONCAT(CL_CONCAT_IMPL (a, b), c)
#define CL_CONCAT4(a, b, c, d) CL_CONCAT(CL_CONCAT (CL_CONCAT_IMPL (a, b), c), d)

#if defined (__OPENCL_VERSION__)
// Declares an integer constant
#define CONSTANT(name, value) enum { name = value }
// A static assertion
#define STATIC_ASSERT(value) void static_assert_method (char [(value) ? 1 : -1])
// Declares a global integer constant. The value of this constant is available
// to the host side after compiling the opencl code.
#define GLOBAL_CONSTANT(name, type, value)                              \
  CONSTANT (name, (type) value);                                        \
  __attribute__ ((reqd_work_group_size (((ulong) name) >> 48 | 0x1000000, \
                                        (((ulong) name) >> 24) & 0xffffff | 0x1000000, \
                                        ((ulong) name) & 0xffffff | 0x1000000))) \
  __kernel void CL_CONCAT4 (globalConstant_, type, _Name_, name) (void) {}
#endif

#endif // !OPENCL_COMMON_H_INCLUDED
