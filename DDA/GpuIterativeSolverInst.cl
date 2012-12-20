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

__kernel void CL_CONCAT(setupPvec__, FLOAT) (__global CFLOAT* pvecX,
                         __global const uchar* materials,
                         __constant CFLOAT* ccSqrt,
                         uint nvCount, uint stride) {
  __global CFLOAT* pvecY = pvecX + stride;
  __global CFLOAT* pvecZ = pvecY + stride;
  for (uint i = get_global_id (0); i < nvCount; i += get_global_size (0)) {
    // Load einc from pvec
    CFLOAT vX = pvecX[i];
    CFLOAT vY = pvecY[i];
    CFLOAT vZ = pvecZ[i];

    uint material = materials[i];
    CFLOAT matX = ccSqrt[material * 3];
    CFLOAT matY = ccSqrt[material * 3 + 1];
    CFLOAT matZ = ccSqrt[material * 3 + 2];
    CFLOAT resX = CFLOAT_(mul) (vX, matX);
    CFLOAT resY = CFLOAT_(mul) (vY, matY);
    CFLOAT resZ = CFLOAT_(mul) (vZ, matZ);
    pvecX[i] = resX;//incPolX;//CFLOAT_(mul) (expV, CFLOAT_(new) (incPolX, 0));
    pvecY[i] = resY;
    pvecZ[i] = resZ;
  }
}

__kernel void CL_CONCAT(setupXvecStartValue__, FLOAT) (__global CFLOAT* xvecX,
                         __global const CFLOAT* startX,
                         __global const uchar* materials,
                         __constant CFLOAT* ccSqrt,
                         uint nvCount, uint stride) {
  __global CFLOAT* xvecY = xvecX + stride;
  __global CFLOAT* xvecZ = xvecY + stride;
  __global const CFLOAT* startY = startX + stride;
  __global const CFLOAT* startZ = startY + stride;
  for (uint i = get_global_id (0); i < nvCount; i += get_global_size (0)) {
    CFLOAT vX = startX[i];
    CFLOAT vY = startY[i];
    CFLOAT vZ = startZ[i];

    uint material = materials[i];
    CFLOAT matX = ccSqrt[material * 3];
    CFLOAT matY = ccSqrt[material * 3 + 1];
    CFLOAT matZ = ccSqrt[material * 3 + 2];
    CFLOAT resX = CFLOAT_(div) (vX, matX);
    CFLOAT resY = CFLOAT_(div) (vY, matY);
    CFLOAT resZ = CFLOAT_(div) (vZ, matZ);
    xvecX[i] = resX;//incPolX;//CFLOAT_(mul) (expV, CFLOAT_(new) (incPolX, 0));
    xvecY[i] = resY;
    xvecZ[i] = resZ;
  }
}

__kernel void CL_CONCAT(getResult__, FLOAT) (__global CFLOAT* resPX,
                         __global const uchar* materials,
                         __constant CFLOAT* ccSqrt,
                         __global const CFLOAT* xvecX,
                         uint nvCount, uint stride) {
  __global CFLOAT* resPY = resPX + stride;
  __global CFLOAT* resPZ = resPY + stride;
  __global const CFLOAT* xvecY = xvecX + stride;
  __global const CFLOAT* xvecZ = xvecY + stride;
  for (uint i = get_global_id (0); i < nvCount; i += get_global_size (0)) {
    //Vector3<ctype> xvecv = g ().get (xvec, i);
    //Vector3<ctype> value = g ().cc_sqrt ()[dipoleGeometry ().getMaterialIndex (i)] * xvec;
    //resP[i] = value;

    CFLOAT xX = xvecX[i];
    CFLOAT xY = xvecY[i];
    CFLOAT xZ = xvecZ[i];

    uint material = materials[i];
    CFLOAT matX = ccSqrt[material * 3];
    CFLOAT matY = ccSqrt[material * 3 + 1];
    CFLOAT matZ = ccSqrt[material * 3 + 2];
    CFLOAT resX = CFLOAT_(mul) (xX, matX);
    CFLOAT resY = CFLOAT_(mul) (xY, matY);
    CFLOAT resZ = CFLOAT_(mul) (xZ, matZ);
    resPX[i] = resX;
    resPY[i] = resY;
    resPZ[i] = resZ;
  }
}

#include <OpenCL/FloatSuffix.h>

// Local Variables: 
// mode: c
// End: 
