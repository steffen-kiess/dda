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

// Block = block of (normally 16) threads inside a work-group
inline uint getBlockId (uint blockSize) {
  // Bug in NVidia implementation: get_local_id () should return
  // size_t (6.11.1), which is unsigned (6.1.1)
  //return get_local_id (0) / i;
  return (size_t) get_local_id (0) / blockSize;
}
inline uint getGlobalBlockId (uint blockSize) {
  return (size_t) get_global_id (0) / blockSize;
}
inline uint getThreadId (uint blockSize) {
  return (size_t) get_local_id (0) % blockSize;
}

#define NR 8
#include "GpuTransposePlanInst.cl"
#define NR 16
#include "GpuTransposePlanInst.cl"

// Local Variables: 
// mode: c
// End: 
