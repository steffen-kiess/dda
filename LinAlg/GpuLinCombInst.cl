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

FLOAT FLOAT_(sum_wg) (__local FLOAT* lvaluep, FLOAT value) {
  //__local FLOAT lvalue;
  //__local FLOAT* lvaluep = &lvalue;

  if (!get_local_id (0))
    *lvaluep = 0;
  barrier (CLK_LOCAL_MEM_FENCE);
  for (size_t i = 0; i < get_local_size (0); i++) {
    if (get_local_id (0) == i)
      *lvaluep += value;
    barrier (CLK_LOCAL_MEM_FENCE);
  }
  return *lvaluep;
}

__kernel void CL_CONCAT(reduceSqnOne__, FLOAT) (ulong countL, __global FLOAT* temp,
                                    __global const CFLOAT* input1) {
  __local FLOAT lvalue;

  size_t count = (size_t) countL;

  FLOAT sum = 0;
  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    sum += CFLOAT_(squared_norm) (input1[i]);
  }  

  sum = FLOAT_(sum_wg) (&lvalue, sum);
  if (!get_local_id (0))
    temp[get_group_id (0)] = sum;
}
__kernel void CL_CONCAT(reduceSqnOne2__, FLOAT) (ulong countL, __global const FLOAT* temp,
                                    __global /*FLOAT*/ char* outB, ulong outO) {
  __local FLOAT lvalue;

  size_t count = (size_t) countL;

  __global FLOAT* out = (__global FLOAT*)(outB + outO);

  FLOAT sum = 0;
  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    sum += temp[i];
  }  

  sum = FLOAT_(sum_wg) (&lvalue, sum);
  if (!get_local_id (0))
    *out = sum;
}

__kernel void CL_CONCAT(reduceVecOne__, FLOAT) (ulong countL, __global FLOAT* temp,
                                    __global const CFLOAT* input1) {
  __local FLOAT lvalue;

  size_t count = (size_t) countL;

  FLOAT sumRRII = 0;
  FLOAT sumRI = 0;
  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    CFLOAT val = input1[i];
    sumRRII += CFLOAT_(real) (val) * CFLOAT_(real) (val) - CFLOAT_(imag) (val) * CFLOAT_(imag) (val);
    sumRI += CFLOAT_(real) (val) * CFLOAT_(imag) (val);
  }  

  sumRRII = FLOAT_(sum_wg) (&lvalue, sumRRII);
  sumRI = FLOAT_(sum_wg) (&lvalue, sumRI);
  if (!get_local_id (0)) {
    temp[get_group_id (0)] = sumRRII;
    temp[get_group_id (0) + get_num_groups (0)] = sumRI;
  }
}
__kernel void CL_CONCAT(reduceVecOne2__, FLOAT) (ulong countL, __global const FLOAT* temp,
                                    __global /*CFLOAT*/ char* outVB, ulong outVO) {
  __local FLOAT lvalue;

  size_t count = (size_t) countL;

  __global CFLOAT* outV = (__global CFLOAT*)(outVB + outVO);

  FLOAT sumRRII = 0;
  FLOAT sumRI = 0;
  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    sumRRII += temp[i];
    sumRI += temp[i + count];
  }  

  sumRRII = FLOAT_(sum_wg) (&lvalue, sumRRII);
  sumRI = FLOAT_(sum_wg) (&lvalue, sumRI);
  if (!get_local_id (0)) {
    *outV = CFLOAT_(new) (sumRRII, 2 * sumRI);
  }
}

__kernel void CL_CONCAT(reduceSqnVecOne__, FLOAT) (ulong countL, __global FLOAT* temp,
                                    __global const CFLOAT* input1) {
  __local FLOAT lvalue;

  size_t count = (size_t) countL;

  FLOAT sumRR = 0;
  FLOAT sumII = 0;
  FLOAT sumRI = 0;
  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    CFLOAT val = input1[i];
    sumRR += CFLOAT_(real) (val) * CFLOAT_(real) (val);
    sumII += CFLOAT_(imag) (val) * CFLOAT_(imag) (val);
    sumRI += CFLOAT_(real) (val) * CFLOAT_(imag) (val);
  }  

  sumRR = FLOAT_(sum_wg) (&lvalue, sumRR);
  sumII = FLOAT_(sum_wg) (&lvalue, sumII);
  sumRI = FLOAT_(sum_wg) (&lvalue, sumRI);
  if (!get_local_id (0)) {
    temp[get_group_id (0)] = sumRR;
    temp[get_group_id (0) + get_num_groups (0)] = sumII;
    temp[get_group_id (0) + 2 * get_num_groups (0)] = sumRI;
  }
}
__kernel void CL_CONCAT(reduceSqnVecOne2__, FLOAT) (ulong countL, __global const FLOAT* temp,
                                       __global /*FLOAT*/ char* outB, ulong outO,
                                       __global /*CFLOAT*/ char* outVB, ulong outVO) {
  __local FLOAT lvalue;

  size_t count = (size_t) countL;

  __global FLOAT* out = (__global FLOAT*)(outB + outO);
  __global CFLOAT* outV = (__global CFLOAT*)(outVB + outVO);

  FLOAT sumRR = 0;
  FLOAT sumII = 0;
  FLOAT sumRI = 0;
  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    sumRR += temp[i];
    sumII += temp[i + count];
    sumRI += temp[i + 2 * count];
  }  

  sumRR = FLOAT_(sum_wg) (&lvalue, sumRR);
  sumII = FLOAT_(sum_wg) (&lvalue, sumII);
  sumRI = FLOAT_(sum_wg) (&lvalue, sumRI);
  if (!get_local_id (0)) {
    *out = sumRR + sumII;
    *outV = CFLOAT_(new) (sumRR - sumII, 2 * sumRI);
  }
}

__kernel void CL_CONCAT(linCombOne__, CFLOAT) (ulong countL, __global CFLOAT* output,
                                  __global const CFLOAT* input1) {
  size_t count = (size_t) countL;

  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    output[i] = input1[i];
  }
}

__kernel void CL_CONCAT(linCombCp__, CFLOAT) (ulong countL, __global CFLOAT* output,
                                    __global const CFLOAT* input1, __global const /*CFLOAT*/ char* scale1B, ulong scale1O) {
  size_t count = (size_t) countL;

  __local CFLOAT scale1L;
  if (!get_local_id (0))
    scale1L = *(__global const CFLOAT*)(scale1B + scale1O);
  barrier (CLK_LOCAL_MEM_FENCE);
  CFLOAT scale1 = scale1L;

  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    output[i] = CFLOAT_(mul) (input1[i], scale1);
  }
}

__kernel void CL_CONCAT(linCombNegOne__, CFLOAT) (ulong countL, __global CFLOAT* output,
                                    __global const CFLOAT* input1,
                                    __global const CFLOAT* input2) {
  size_t count = (size_t) countL;

  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    output[i] = CFLOAT_(sub) (input2[i], input1[i]);
  }
}

__kernel void CL_CONCAT(linCombRpOne__, CFLOAT) (ulong countL, __global CFLOAT* output,
                                    __global const CFLOAT* input1, __global const /*FLOAT*/ char* scale1B, ulong scale1O,
                                    __global const CFLOAT* input2) {
  size_t count = (size_t) countL;

  __local FLOAT scale1L;
  if (!get_local_id (0))
    scale1L = *(__global const FLOAT*)(scale1B + scale1O);
  barrier (CLK_LOCAL_MEM_FENCE);
  FLOAT scale1 = scale1L;

  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    output[i] = CFLOAT_(add) (CFLOAT_(mul_real) (input1[i], scale1), input2[i]);
  }
}

__kernel void CL_CONCAT(linCombCpOne__, CFLOAT) (ulong countL, __global CFLOAT* output,
                                    __global const CFLOAT* input1, __global const /*CFLOAT*/ char* scale1B, ulong scale1O,
                                    __global const CFLOAT* input2) {
  size_t count = (size_t) countL;

  __local CFLOAT scale1L;
  if (!get_local_id (0))
    scale1L = *(__global const CFLOAT*)(scale1B + scale1O);
  barrier (CLK_LOCAL_MEM_FENCE);
  CFLOAT scale1 = scale1L;

  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    output[i] = CFLOAT_(add) (CFLOAT_(mul) (input1[i], scale1), input2[i]);
  }
}

__kernel void CL_CONCAT(linCombRpCp__, CFLOAT) (ulong countL, __global CFLOAT* output,
                                   __global const CFLOAT* input1, __global const /*FLOAT*/ char* scale1B, ulong scale1O,
                                   __global const CFLOAT* input2, __global const /*CFLOAT*/ char* scale2B, ulong scale2O) {
  size_t count = (size_t) countL;

  __local FLOAT scale1L;
  if (!get_local_id (0))
    scale1L = *(__global const FLOAT*)(scale1B + scale1O);
  __local CFLOAT scale2L;
  if (!get_local_id (0))
    scale2L = *(__global const CFLOAT*)(scale2B + scale2O);
  barrier (CLK_LOCAL_MEM_FENCE);
  FLOAT scale1 = scale1L;
  CFLOAT scale2 = scale2L;

  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    output[i] = CFLOAT_(add) (CFLOAT_(mul_real) (input1[i], scale1), CFLOAT_(mul) (input2[i], scale2));
  }
}

__kernel void CL_CONCAT(linCombCpCp__, CFLOAT) (ulong countL, __global CFLOAT* output,
                                   __global const CFLOAT* input1, __global const /*CFLOAT*/ char* scale1B, ulong scale1O,
                                   __global const CFLOAT* input2, __global const /*CFLOAT*/ char* scale2B, ulong scale2O) {
  size_t count = (size_t) countL;

  __local CFLOAT scale1L;
  if (!get_local_id (0))
    scale1L = *(__global const CFLOAT*)(scale1B + scale1O);
  __local CFLOAT scale2L;
  if (!get_local_id (0))
    scale2L = *(__global const CFLOAT*)(scale2B + scale2O);
  barrier (CLK_LOCAL_MEM_FENCE);
  CFLOAT scale1 = scale1L;
  CFLOAT scale2 = scale2L;

  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    output[i] = CFLOAT_(add) (CFLOAT_(mul) (input1[i], scale1), CFLOAT_(mul) (input2[i], scale2));
  }
}

__kernel void CL_CONCAT(linCombCpCpOne__, CFLOAT) (ulong countL, __global CFLOAT* output,
                                   __global const CFLOAT* input1, __global const /*CFLOAT*/ char* scale1B, ulong scale1O,
                                   __global const CFLOAT* input2, __global const /*CFLOAT*/ char* scale2B, ulong scale2O,
                                   __global const CFLOAT* input3) {
  size_t count = (size_t) countL;

  __local CFLOAT scale1L;
  if (!get_local_id (0))
    scale1L = *(__global const CFLOAT*)(scale1B + scale1O);
  __local CFLOAT scale2L;
  if (!get_local_id (0))
    scale2L = *(__global const CFLOAT*)(scale2B + scale2O);
  barrier (CLK_LOCAL_MEM_FENCE);
  CFLOAT scale1 = scale1L;
  CFLOAT scale2 = scale2L;

  for (size_t i = get_global_id (0); i < count; i += get_global_size (0))
    output[i] = CFLOAT_(add) (CFLOAT_(add) (CFLOAT_(mul) (input1[i], scale1), CFLOAT_(mul) (input2[i], scale2)), input3[i]);
}

__kernel void CL_CONCAT(linCombCpCpCp__, CFLOAT) (ulong countL, __global CFLOAT* output,
                                   __global const CFLOAT* input1, __global const /*CFLOAT*/ char* scale1B, ulong scale1O,
                                   __global const CFLOAT* input2, __global const /*CFLOAT*/ char* scale2B, ulong scale2O,
                                   __global const CFLOAT* input3, __global const /*CFLOAT*/ char* scale3B, ulong scale3O) {
  size_t count = (size_t) countL;

  __local CFLOAT scale1L;
  if (!get_local_id (0))
    scale1L = *(__global const CFLOAT*)(scale1B + scale1O);
  __local CFLOAT scale2L;
  if (!get_local_id (0))
    scale2L = *(__global const CFLOAT*)(scale2B + scale2O);
  __local CFLOAT scale3L;
  if (!get_local_id (0))
    scale3L = *(__global const CFLOAT*)(scale3B + scale3O);
  barrier (CLK_LOCAL_MEM_FENCE);
  CFLOAT scale1 = scale1L;
  CFLOAT scale2 = scale2L;
  CFLOAT scale3 = scale3L;

  for (size_t i = get_global_id (0); i < count; i += get_global_size (0)) {
    output[i] = CFLOAT_(add) (CFLOAT_(add) (CFLOAT_(mul) (input1[i], scale1),
                                        CFLOAT_(mul) (input2[i], scale2)),
                            CFLOAT_(mul) (input3[i], scale3));
  }
}

#include <OpenCL/FloatSuffix.h>

// Local Variables: 
// mode: c
// End: 
