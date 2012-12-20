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

// This file is included twice, for single and double precision

#include <OpenCL/FloatPrefix.h>

__kernel void CL_CONCAT(calcField__, FLOAT) (__global const uchar* valid, __global const uint* positionsX, __global const CFLOAT* pvecX, uint nvCount, uint stride, __global CFLOAT* res, FLOAT kd, FLOAT nx, FLOAT ny, FLOAT nz) {
  __global const uint* positionsY = positionsX + stride;
  __global const uint* positionsZ = positionsY + stride;
  __global const CFLOAT* pvecY = pvecX + stride;
  __global const CFLOAT* pvecZ = pvecY + stride;
  CFLOAT sumX = CFLOAT_(new) (0, 0);
  CFLOAT sumY = CFLOAT_(new) (0, 0);
  CFLOAT sumZ = CFLOAT_(new) (0, 0);
  for (uint i = get_global_id (0); i < nvCount; i += get_global_size (0)) {
    if (valid[i]) {
      uint x = positionsX[i];
      uint y = positionsY[i];
      uint z = positionsZ[i];
      CFLOAT valX = pvecX[i];
      CFLOAT valY = pvecY[i];
      CFLOAT valZ = pvecZ[i];
      FLOAT arg = -kd * (nx * x + ny * y + nz * z);
      CFLOAT a = CFLOAT_(new) (cos (arg), sin (arg));
      sumX = CFLOAT_(add) (sumX, CFLOAT_(mul) (valX, a));
      sumY = CFLOAT_(add) (sumY, CFLOAT_(mul) (valY, a));
      sumZ = CFLOAT_(add) (sumZ, CFLOAT_(mul) (valZ, a));
    }
  }
  
  __local CFLOAT lvalueX, lvalueY, lvalueZ;
  if (get_local_id (0) == 0)
    lvalueX = lvalueY = lvalueZ = CFLOAT_(new) (0, 0);
  barrier (CLK_LOCAL_MEM_FENCE);
  for (size_t i = 0; i < get_local_size (0); i++) {
    if (get_local_id (0) == i) {
      lvalueX = CFLOAT_(add) (lvalueX, sumX);
      lvalueY = CFLOAT_(add) (lvalueY, sumY);
      lvalueZ = CFLOAT_(add) (lvalueZ, sumZ);
    }
    barrier (CLK_LOCAL_MEM_FENCE);
  }
  if (get_local_id (0) == 0) {
    res[get_group_id (0)] = lvalueX;
    res[get_group_id (0) + get_num_groups (0)] = lvalueY;
    res[get_group_id (0) + 2 * get_num_groups (0)] = lvalueZ;
  }
  barrier (CLK_LOCAL_MEM_FENCE);
}

#include <OpenCL/FloatSuffix.h>

// Local Variables: 
// mode: c
// End: 
