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

// This file is included twice, for single and double precision, and has no include guards

#include <OpenCL/FloatPrefix.h>

#if defined (__OPENCL_VERSION__)
typedef CL_CONCAT(FLOAT, 2) CFLOAT;
inline CFLOAT CFLOAT_(new) (FLOAT re, FLOAT im) { return (CFLOAT){re, im}; }
inline FLOAT CFLOAT_(real) (CFLOAT c) { return c.x; }
inline FLOAT CFLOAT_(imag) (CFLOAT c) { return c.y; }
#else
#if defined (__cplusplus)
#include <complex>
typedef std::complex<FLOAT> CFLOAT;
inline CFLOAT CFLOAT_(new) (FLOAT re, FLOAT im) { return CFLOAT (re, im); }
inline FLOAT CFLOAT_(real) (CFLOAT c) { return c.real (); }
inline FLOAT CFLOAT_(imag) (CFLOAT c) { return c.imag (); }
#else
typedef struct CFLOAT_(struct) {
  FLOAT re, im;
} CFLOAT;
inline CFLOAT CFLOAT_(new) (FLOAT re, FLOAT im) { CFLOAT r; r.re = re; r.im = im; return r; }
inline FLOAT CFLOAT_(real) (CFLOAT c) { return c.re; }
inline FLOAT CFLOAT_(imag) (CFLOAT c) { return c.im; }
#endif
#endif

inline FLOAT FLOAT_(mul) (FLOAT a, FLOAT b) {
  return a * b;
}
inline FLOAT FLOAT_(div) (FLOAT a, FLOAT b) {
  return a / b;
}
inline FLOAT FLOAT_(add) (FLOAT a, FLOAT b) {
  return a + b;
}
inline FLOAT FLOAT_(sub) (FLOAT a, FLOAT b) {
  return a - b;
}
inline FLOAT FLOAT_(conj) (FLOAT a) {
  return a;
}

inline CFLOAT CFLOAT_(mul) (CFLOAT a, CFLOAT b) {
  return CFLOAT_(new) (CFLOAT_(real) (a) * CFLOAT_(real) (b) - CFLOAT_(imag) (a) * CFLOAT_(imag) (b),
                      CFLOAT_(real) (a) * CFLOAT_(imag) (b) + CFLOAT_(imag) (a) * CFLOAT_(real) (b));
}
inline CFLOAT CFLOAT_(mul_real) (CFLOAT a, FLOAT b) {
  return a * b;
}
inline CFLOAT CFLOAT_(add) (CFLOAT a, CFLOAT b) {
  return a + b;
}
inline CFLOAT CFLOAT_(sub) (CFLOAT a, CFLOAT b) {
  return a - b;
}
inline CFLOAT CFLOAT_(conj) (CFLOAT a) {
  return CFLOAT_(new) (CFLOAT_(real) (a), -CFLOAT_(imag) (a));
}
inline FLOAT CFLOAT_(squared_norm) (CFLOAT a) {
  // a.real * a.real + a.imag * a.imag
#if defined (__OPENCL_VERSION__)
  CFLOAT b = a * a; // (a.real * a.real, a.imag * a.imag)
  return CFLOAT_(real) (b) + CFLOAT_(imag) (b);
#else
  return CFLOAT_(real) (a) * CFLOAT_(real) (a) + CFLOAT_(imag) (a) * CFLOAT_(imag) (a);
#endif
}
inline CFLOAT CFLOAT_(div) (CFLOAT a, CFLOAT b) {
  FLOAT denominator = CFLOAT_(squared_norm) (b);
  return CFLOAT_(new) (CFLOAT_(real) (a) * CFLOAT_(real) (b) + CFLOAT_(imag) (a) * CFLOAT_(imag) (b),
                       CFLOAT_(imag) (a) * CFLOAT_(real) (b) - CFLOAT_(real) (a) * CFLOAT_(imag) (b)) / denominator;
}

#include <OpenCL/FloatSuffix.h>
