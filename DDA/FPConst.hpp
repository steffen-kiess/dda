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

#ifndef DDA_FPCONST_HPP_INCLUDED
#define DDA_FPCONST_HPP_INCLUDED

// Some floating point constants used by the DDA implementation

#include <Core/Util.hpp>

#include <limits>

#include <boost/static_assert.hpp>

namespace DDA {
#define CONSTS                                                          \
  D (pi,                     3.1415926535897932384626433832795028);     \
  D (two_pi,                 6.2831853071795864769252867665590056);     \
  D (four_pi,               12.5663706143591729538505735331180112);     \
  D (three_over_four_pi,     0.23873241463784300365332564505877);       \
  D (speed_of_light, 299792458.0);                                      \
  D (one,                    1.0);                                      \
  D (zero,                   0.0);                                      \
  D (minus_one,             -1.0);                                      \
  D (two,                    2.0);                                      \
  D (three,                  3.0);                                      \
  D (four,                   4.0);                                      \
  D (one_third,              1.0 / 3.0);                                \
  D (ldr_b1,                 1.8915316);                                \
  D (ldr_b2,                -0.1648469);                                \
  D (ldr_b3,                 1.7700004);                                \

  // ldr_b* is from doi:10.1086/172396

  template <typename F> struct FPConst {
    typedef std::numeric_limits<F> numeric_limits;

    BOOST_STATIC_ASSERT (numeric_limits::is_specialized);
    BOOST_STATIC_ASSERT (!numeric_limits::is_integer);

#define D(name, value) static const F name
    CONSTS
#undef D
  };

#if GCC_VERSION_IS_ATLEAST (4, 4)
  // Use suffix for __float128 (see 
  // http://gcc.gnu.org/onlinedocs/gcc/Floating-Types.html )
  // Only supported starting with gcc-4.3
  // Disabling the warning / error for the suffix is only supported starting with
  // gcc-4.4
#define SUFFIX q
  // Disable warnings for using the suffix. Unfortunatly __extension__ doesn't
  // work here in g++ (only in gcc), see
  // http://gcc.gnu.org/bugzilla/show_bug.cgi?id=7263
#pragma GCC system_header
#else
#define SUFFIX l
#endif
  //#define D3(name, value, suffix) template <typename F> const F FPConst<F>::name = static_cast<F> (value##suffix)
#define D3(name, value, suffix) template <typename F> const F FPConst<F>::name = static_cast<F> (value##suffix)
#define D2(name, value, suffix) D3 (name, value, suffix)
#define D(name, value) D2 (name, value, SUFFIX)
  CONSTS
#undef D
#undef D2
#undef SUFFIX


#undef CONSTS
}

#endif // !DDA_FPCONST_HPP_INCLUDED
