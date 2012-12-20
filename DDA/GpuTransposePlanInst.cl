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

// This file is included twice, with NR = 8 and NR = 16

#define N(x) CL_CONCAT3 (x, __, NR)

#if NR == 4
#define TY uint
#elif NR == 8
#define TY ulong
#elif NR == 16
#define TY ulong2
#else
#error
#endif
__kernel void N(transpose1) (__global char* in, uint inOffset, uint inStride, __global char* out, uint outOffset, uint outStride, uint count) {
  for (uint i = get_global_id (0); i < count; i += get_global_size (0))
    *((__global TY*) (((__global char*) out) + outStride * i + outOffset)) = *((__global TY*) (((__global char*) in) + inStride * i + inOffset));
}
#undef TY

GLOBAL_CONSTANT (N(transpose2_workGroupSize), size_t, 32);
GLOBAL_CONSTANT (N(transpose2_globalSize), size_t, 512 * 16);

CONSTANT (N(transpose2_elementSize), NR);

GLOBAL_CONSTANT (N(transpose2_threadsPerMem), size_t, 16); // Threads doing 1 coalesced memory access
//typedef ulong N(transpose2_pieceType);
typedef uint N(transpose2_pieceType);


STATIC_ASSERT (N(transpose2_globalSize) % N(transpose2_workGroupSize) == 0);
STATIC_ASSERT (N(transpose2_globalSize) > 0);
STATIC_ASSERT (N(transpose2_workGroupSize) > 0);
STATIC_ASSERT (N(transpose2_workGroupSize) % N(transpose2_threadsPerMem) == 0);
CONSTANT (N(transpose2_blocksPerWG), N(transpose2_workGroupSize) / N(transpose2_threadsPerMem));

CONSTANT (N(transpose2_pieceSize), sizeof (N(transpose2_pieceType)));
CONSTANT (N(transpose2_memAccessSize), N(transpose2_threadsPerMem) * N(transpose2_pieceSize));
STATIC_ASSERT (N(transpose2_memAccessSize) % N(transpose2_elementSize) == 0);
GLOBAL_CONSTANT (N(transpose2_elementsPerMem), size_t, N(transpose2_memAccessSize) / N(transpose2_elementSize));

STATIC_ASSERT (N(transpose2_elementSize) % N(transpose2_pieceSize) == 0);
CONSTANT (N(transpose2_piecesPerElement), N(transpose2_elementSize) / N(transpose2_pieceSize));
typedef N(transpose2_pieceType) N(transpose2_elementArType)[N(transpose2_piecesPerElement)];
void N(transposeBlock)
(__local N(transpose2_elementArType) bufferS[N(transpose2_blocksPerWG)][N(transpose2_elementsPerMem) * N(transpose2_elementsPerMem)],
 __global ulong* in, uint inOffset, uint inStride1, uint inStride2,
 __global ulong* out, uint outOffset, uint outStride1, uint outStride2,
 uint count1, uint count2, uint i, uint j) {
  __local N(transpose2_elementArType)* buffer = bufferS[getBlockId (N(transpose2_threadsPerMem))];
  uint xx = getThreadId (N(transpose2_threadsPerMem)) / N(transpose2_piecesPerElement);
  __global char* baseAddr = ((__global char*) in) + inStride1 * (N(transpose2_elementsPerMem) * i + xx) + inStride2 * N(transpose2_elementsPerMem) * j + inOffset;
  for (uint y = 0; y < N(transpose2_elementsPerMem); y++) {
    STATIC_ASSERT (N(transpose2_threadsPerMem) % N(transpose2_piecesPerElement) == 0);
    uint x = getThreadId (N(transpose2_threadsPerMem)) / N(transpose2_piecesPerElement);
    uint piece = getThreadId (N(transpose2_threadsPerMem)) % N(transpose2_piecesPerElement);

    uint i2 = i * N(transpose2_elementsPerMem) + x;
    uint j2 = j * N(transpose2_elementsPerMem) + y;

    if (i2 < count1 && j2 < count2)
      buffer[x * N(transpose2_elementsPerMem) + y][piece] = ((__global N(transpose2_pieceType)*) (baseAddr))[piece];
    baseAddr += inStride2;
  }

  // On NVIDIA CUDA CPUs this also could be a mem_fence instead of a barrier as long as all threads are in the same warp
  barrier (CLK_LOCAL_MEM_FENCE);

  for (uint x = 0; x < N(transpose2_elementsPerMem); x++) {
    STATIC_ASSERT (N(transpose2_threadsPerMem) % N(transpose2_piecesPerElement) == 0);
    uint y = getThreadId (N(transpose2_threadsPerMem)) / N(transpose2_piecesPerElement);
    uint piece = getThreadId (N(transpose2_threadsPerMem)) % N(transpose2_piecesPerElement);

    uint i2 = i * N(transpose2_elementsPerMem) + x;
    uint j2 = j * N(transpose2_elementsPerMem) + y;

    if (i2 < count1 && j2 < count2)
      ((__global N(transpose2_pieceType)*) (((__global char*) out) + outStride1 * i2 + outStride2 * j2 + outOffset))[piece] = buffer[x * N(transpose2_elementsPerMem) + y][piece];
  }

  barrier (CLK_LOCAL_MEM_FENCE);
}

__attribute__((reqd_work_group_size(0 + N(transpose2_workGroupSize), 1, 1)))
__kernel void N(transpose2) 
(__global ulong* in, uint inOffset, uint inStride1, uint inStride2,
 __global ulong* out, uint outOffset, uint outStride1, uint outStride2,
 uint count1, uint count2) {
  __local N(transpose2_elementArType) bufferS[N(transpose2_blocksPerWG)][N(transpose2_elementsPerMem) * N(transpose2_elementsPerMem)];
  uint blockCount1 = (count1 + N(transpose2_elementsPerMem) - 1) / N(transpose2_elementsPerMem);
  uint blockCount2 = (count2 + N(transpose2_elementsPerMem) - 1) / N(transpose2_elementsPerMem);
#if 0
#ifdef cl_amd_printf
#pragma OPENCL EXTENSION cl_amd_printf: enable
  if (!get_global_id (0))
    printf ("\nget_global_id (0) = %d, incr = %d, blockCount1 = %d, blockCount2 = %d\n", get_global_id (0), globalSize / threadsPerMem, blockCount1, blockCount2);
#endif
#endif
  uint myBlock = getGlobalBlockId (N(transpose2_threadsPerMem));
  uint pos1 = myBlock % blockCount1;
  myBlock /= blockCount1;
  uint pos2 = myBlock;

  uint inc = N(transpose2_globalSize) / N(transpose2_threadsPerMem);
  uint inc1 = inc % blockCount1;
  inc /= blockCount1;
  uint inc2 = inc;

  bool done = pos2 >= blockCount2;
  while (!done) {
    N(transposeBlock) (bufferS,
                       in, inOffset, inStride1, inStride2,
                       out, outOffset, outStride1, outStride2,
                       count1, count2, pos1, pos2);
    pos1 += inc1;
    if (pos1 >= blockCount1) {
      pos1 -= blockCount1;
      pos2++;
    }
    pos2 += inc2;
    if (pos2 >= blockCount2)
      done = true;
  }
}

#if 0
__attribute__((reqd_work_group_size(0 + N(transpose2_workGroupSize), 1, 1)))
__kernel void N(transpose3)
(__global ulong* in, uint inOffset, uint inStride1, uint inStride2, uint inStride3,
 __global ulong* out, uint outOffset, uint outStride1, uint outStride2, uint outStride3,
 uint count1, uint count2, uint count3, uint inc1, uint inc2, uint inc3) {
  for (uint i = 0; i < count3; i++)
    N(transpose2) (in, inOffset + i * inStride3, inStride1, inStride2,
                   out, outOffset + i * outStride3, outStride1, outStride2,
                   count1, count2);
}

#else

__attribute__((reqd_work_group_size(0 + N(transpose2_workGroupSize), 1, 1)))
__kernel void N(transpose3)
(__global ulong* in, uint inOffset, uint inStride1, uint inStride2, uint inStride3,
 __global ulong* out, uint outOffset, uint outStride1, uint outStride2, uint outStride3,
 uint count1, uint count2, uint count3, uint inc1, uint inc2, uint inc3) {
  __local N(transpose2_elementArType) bufferS[N(transpose2_blocksPerWG)][N(transpose2_elementsPerMem) * N(transpose2_elementsPerMem)];
  uint blockCount1 = (count1 + N(transpose2_elementsPerMem) - 1) / N(transpose2_elementsPerMem);
  uint blockCount2 = (count2 + N(transpose2_elementsPerMem) - 1) / N(transpose2_elementsPerMem);
#if 0
#ifdef cl_amd_printf
#pragma OPENCL EXTENSION cl_amd_printf: enable
  if (!get_global_id (0))
    printf ("\nget_global_id (0) = %d, incr = %d, blockCount1 = %d, blockCount2 = %d\n", get_global_id (0), globalSize / threadsPerMem, blockCount1, blockCount2);
#endif
#endif
  uint myBlock = getGlobalBlockId (N(transpose2_threadsPerMem));
  uint pos1 = myBlock % blockCount1;
  myBlock /= blockCount1;
  uint pos2 = myBlock % blockCount2;
  myBlock /= blockCount2;
  uint pos3 = myBlock;

  bool done = pos3 >= count3;
  while (!done) {
    N(transposeBlock) (bufferS,
                       in, inOffset + pos3 * inStride3, inStride1, inStride2,
                       out, outOffset + pos3 * outStride3, outStride1, outStride2,
                       count1, count2, pos1, pos2);
    pos1 += inc1;
    if (pos1 >= blockCount1) {
      pos1 -= blockCount1;
      pos2++;
    }
    pos2 += inc2;
    if (pos2 >= blockCount2) {
      pos2 -= blockCount2;
      pos3++;
    }
    pos3 += inc3;
    if (pos3 >= count3)
      done = true;
  }
}
#endif

#if 0
__attribute__((reqd_work_group_size(0 + N(transpose2_workGroupSize), 1, 1)))
__kernel void N(transpose4) 
(__global ulong* in, uint inOffset, uint inStride1, uint inStride2, uint inStride3, uint inStride4,
 __global ulong* out, uint outOffset, uint outStride1, uint outStride2, uint outStride3, uint outStride4,
 uint count1, uint count2, uint count3, uint count4) {
  for (uint i = 0; i < count4; i++)
    N(transpose3) (in, inOffset + i * inStride4, inStride1, inStride2, inStride3,
                   out, outOffset + i * outStride4, outStride1, outStride2, outStride3,
                   count1, count2, count3);
}

#else

__attribute__((reqd_work_group_size(0 + N(transpose2_workGroupSize), 1, 1)))
__kernel void N(transpose4) 
(__global ulong* in, uint inOffset, uint inStride1, uint inStride2, uint inStride3, uint inStride4,
 __global ulong* out, uint outOffset, uint outStride1, uint outStride2, uint outStride3, uint outStride4,
 uint count1, uint count2, uint count3, uint count4) {
  __local N(transpose2_elementArType) bufferS[N(transpose2_blocksPerWG)][N(transpose2_elementsPerMem) * N(transpose2_elementsPerMem)];
  uint blockCount1 = (count1 + N(transpose2_elementsPerMem) - 1) / N(transpose2_elementsPerMem);
  uint blockCount2 = (count2 + N(transpose2_elementsPerMem) - 1) / N(transpose2_elementsPerMem);
#if 0
#ifdef cl_amd_printf
#pragma OPENCL EXTENSION cl_amd_printf: enable
  if (!get_global_id (0))
    printf ("\nget_global_id (0) = %d, incr = %d, blockCount1 = %d, blockCount2 = %d\n", get_global_id (0), globalSize / threadsPerMem, blockCount1, blockCount2);
#endif
#endif
  uint myBlock = getGlobalBlockId (N(transpose2_threadsPerMem));
  uint pos1 = myBlock % blockCount1;
  myBlock /= blockCount1;
  uint pos2 = myBlock % blockCount2;
  myBlock /= blockCount2;
  uint pos3 = myBlock % count3;
  myBlock /= count3;
  uint pos4 = myBlock;

  uint inc = N(transpose2_globalSize) / N(transpose2_threadsPerMem);
  uint inc1 = inc % blockCount1;
  inc /= blockCount1;
  uint inc2 = inc % blockCount2;
  inc /= blockCount2;
  uint inc3 = inc % count3;
  inc /= count3;
  uint inc4 = inc;

  bool done = pos4 >= count4;
  while (!done) {
    N(transposeBlock) (bufferS,
                       in, inOffset + pos4 * inStride4 + pos3 * inStride3, inStride1, inStride2,
                       out, outOffset + pos4 * outStride4 + pos3 * outStride3, outStride1, outStride2,
                       count1, count2, pos1, pos2);
    pos1 += inc1;
    if (pos1 >= blockCount1) {
      pos1 -= blockCount1;
      pos2++;
    }
    pos2 += inc2;
    if (pos2 >= blockCount2) {
      pos2 -= blockCount2;
      pos3++;
    }
    pos3 += inc3;
    if (pos3 >= count3) {
      pos3 -= count3;
      pos4++;
    }
    pos4 += inc4;
    if (pos4 >= count4)
      done = true;
  }
}
#endif

#undef N
#undef NR

// Local Variables: 
// mode: c
// End: 
