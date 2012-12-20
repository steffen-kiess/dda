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

#ifndef __NVCL_OPENCL_COMMON_H__
#pragma OPENCL EXTENSION cl_khr_byte_addressable_store : enable
#endif

__kernel void memset(__global uchar *target, uint offset, uchar c, uint size) {
  for (uint i = get_global_id (0); i < size; i += get_global_size (0))
    target[i] = c;
}

__kernel void memsetZeroInt(__global uint *target, uint offset, uint size) {
  for (uint i = get_global_id (0); i < size; i += get_global_size (0))
    target[i] = 0;
}

__kernel void memoryToPointer (__global uint* target, __global char* value) {
  size_t v = (size_t) value;
  *target = v > UINT_MAX ? 0 : (uint) v;
}

// Local Variables: 
// mode: c
// End: 
