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

inline CFLOAT CL_CONCAT(maybeConj__, FLOAT) (CFLOAT f, bool conj) {
  return conj ? CFLOAT_(conj) (f) : f;
}

__kernel void CL_CONCAT(initXMatrix__, FLOAT) (__global CFLOAT* xMatrixX,
                           __global const uchar* materials,
                           __global const uint* positionsX,
                           __constant CFLOAT* ccSqrt,
                           __global const CFLOAT* arg, uchar conj,
                           uint z0, uint gridX, uint boxY, uint boxZ,
                           uint nvCount, uint stride) {
  __global const uint* positionsY = positionsX + stride;
  __global const uint* positionsZ = positionsY + stride;
  __global CFLOAT* xMatrixY = xMatrixX + gridX * boxY * boxZ;
  __global CFLOAT* xMatrixZ = xMatrixY + gridX * boxY * boxZ;
  for (uint i = get_global_id (0); i < nvCount; i += get_global_size (0)) {
    uint x = positionsX[i];
    uint y = positionsY[i];
    uint z = positionsZ[i];
    uint material = materials[i];
    CFLOAT argX = CL_CONCAT(maybeConj__, FLOAT) (arg[i + 0 * stride], conj);
    CFLOAT argY = CL_CONCAT(maybeConj__, FLOAT) (arg[i + 1 * stride], conj);
    CFLOAT argZ = CL_CONCAT(maybeConj__, FLOAT) (arg[i + 2 * stride], conj);
    CFLOAT matX = ccSqrt[material * 3];
    CFLOAT matY = ccSqrt[material * 3 + 1];
    CFLOAT matZ = ccSqrt[material * 3 + 2];
    CFLOAT resX = CFLOAT_(mul) (argX, matX);
    CFLOAT resY = CFLOAT_(mul) (argY, matY);
    CFLOAT resZ = CFLOAT_(mul) (argZ, matZ);
    uint index = x + gridX * (y + boxY * (z - z0));
    xMatrixX[index] = resX; // these memory accesses not coalesced
    xMatrixY[index] = resY;
    xMatrixZ[index] = resZ;
  }
}

__kernel void CL_CONCAT(createResVec__, FLOAT) (__global const CFLOAT* xMatrixX,
                            __global const uchar* materials,
                            __global const uint* positionsX,
                            __constant CFLOAT* ccSqrt,
                            __global const CFLOAT* arg,
                            __global CFLOAT* res, uchar conj,
                            uint z0, uint gridX, uint boxY, uint boxZ,
                            uint nvCount, uint stride) {
  __global const uint* positionsY = positionsX + stride;
  __global const uint* positionsZ = positionsY + stride;
  __global const CFLOAT* xMatrixY = xMatrixX + gridX * boxY * boxZ;
  __global const CFLOAT* xMatrixZ = xMatrixY + gridX * boxY * boxZ;
  for (uint i = get_global_id (0) + nvCount; i < stride; i += get_global_size (0)) {
    res[i + 0 * stride] = CFLOAT_(new) (0, 0);
    res[i + 1 * stride] = CFLOAT_(new) (0, 0);
    res[i + 2 * stride] = CFLOAT_(new) (0, 0);
  }
  for (uint i = get_global_id (0); i < nvCount; i += get_global_size (0)) {
    uint x = positionsX[i];
    uint y = positionsY[i];
    uint z = positionsZ[i];
    uint material = materials[i];
    uint index = x + gridX * (y + boxY * (z - z0));
    CFLOAT xvalX = xMatrixX[index]; // these memory accesses not coalesced
    CFLOAT xvalY = xMatrixY[index];
    CFLOAT xvalZ = xMatrixZ[index];
    CFLOAT matX = ccSqrt[material * 3];
    CFLOAT matY = ccSqrt[material * 3 + 1];
    CFLOAT matZ = ccSqrt[material * 3 + 2];
    CFLOAT argX = CL_CONCAT(maybeConj__, FLOAT) (arg[i + 0 * stride], conj);
    CFLOAT argY = CL_CONCAT(maybeConj__, FLOAT) (arg[i + 1 * stride], conj);
    CFLOAT argZ = CL_CONCAT(maybeConj__, FLOAT) (arg[i + 2 * stride], conj);
    CFLOAT resX = CFLOAT_(add) (CFLOAT_(mul) (xvalX, matX), argX);
    CFLOAT resY = CFLOAT_(add) (CFLOAT_(mul) (xvalY, matY), argY);
    CFLOAT resZ = CFLOAT_(add) (CFLOAT_(mul) (xvalZ, matZ), argZ);
    res[i + 0 * stride] = CL_CONCAT(maybeConj__, FLOAT) (resX, conj);
    res[i + 1 * stride] = CL_CONCAT(maybeConj__, FLOAT) (resY, conj);
    res[i + 2 * stride] = CL_CONCAT(maybeConj__, FLOAT) (resZ, conj);
  }
}

__kernel void CL_CONCAT(innerLoop__, FLOAT) (__global CFLOAT* slicesTr, __global const CFLOAT* dMatrix, uint si, uint gridX, uint gridY, uint gridZ) {
  uint j = get_global_id (0);
  uint k = get_global_id (1);
  uint slice = get_global_id (2);

  CFLOAT xv[3];
  for (int comp = 0; comp < 3; comp++)
    xv[comp] = slicesTr[j + gridY * (k + gridZ * (comp + 3 * slice))];
  CFLOAT dm[6];
  for (int comp = 0; comp < 6; comp++)
    dm[comp] = dMatrix[comp + 6 * (j + gridY * (k + gridZ * (si + slice)))];
  CFLOAT yv[3];
  yv[0] = CFLOAT_(add) (CFLOAT_(add) (CFLOAT_(mul) (dm[0], xv[0]), CFLOAT_(mul) (dm[1], xv[1])), CFLOAT_(mul) (dm[2], xv[2]));
  yv[1] = CFLOAT_(add) (CFLOAT_(add) (CFLOAT_(mul) (dm[1], xv[0]), CFLOAT_(mul) (dm[3], xv[1])), CFLOAT_(mul) (dm[4], xv[2]));
  yv[2] = CFLOAT_(add) (CFLOAT_(add) (CFLOAT_(mul) (dm[2], xv[0]), CFLOAT_(mul) (dm[4], xv[1])), CFLOAT_(mul) (dm[5], xv[2]));
  for (int comp = 0; comp < 3; comp++)
    slicesTr[j + gridY * (k + gridZ * (comp + 3 * slice))] = yv[comp];
}

#include <OpenCL/FloatSuffix.h>

// Local Variables: 
// mode: c
// End: 
