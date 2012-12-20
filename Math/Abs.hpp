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

#ifndef MATH_ABS_HPP_INCLUDED
#define MATH_ABS_HPP_INCLUDED

// Methods for getting the absolute value (Math::abs()) and the squared absolute
// value (Math::abs2()).
//
// Implementations can be provided by specializing
// Math::AbsImpl<> / Math::Abs2Impl<>
// Math::abs() per default is implemented using Math::abs2()

#include <cstdlib>
#include <cmath>
#include <climits>
#include <complex>

namespace Math {
  template <typename T> struct Abs2Impl {
    static __decltype (std::norm (*(T*)NULL)) apply (T value) {
      return std::norm (value);
    }
  };

  template <typename T> __decltype (Abs2Impl<T>::apply (*(T*)NULL)) abs2 (T value) {
    return Abs2Impl<T>::apply (value);
  }

  template <typename T> struct AbsImpl {
    static __decltype (std::sqrt (Math::abs2 (*(T*)NULL))) apply (T value) {
      return std::sqrt (Math::abs2 (value));
    }
  };

  template <typename T> __decltype (AbsImpl<T>::apply (*(T*)NULL)) abs (T value) {
    return AbsImpl<T>::apply (value);
  }

#define DEF2(T)                                                         \
  template <> struct Abs2Impl<T> {                                      \
    static __decltype ((*(T*)NULL) * (*(T*)NULL)) apply (T value) {     \
      return value * value;                                             \
    }                                                                   \
  };
#define DEF(T)                                                  \
  DEF2(T)                                                       \
  template <> struct AbsImpl<T> {                               \
    static __decltype (std::abs (*(T*)NULL)) apply (T value) {  \
      return std::abs (value);                                  \
    }                                                           \
  }
#define DEFU(T)                                 \
  DEF2(T)                                       \
  template <> struct AbsImpl<T> {               \
    static T apply (T value) {                  \
      return value;                             \
    }                                           \
  }
  DEFU (unsigned char); DEF (signed char);
#if CHAR_MIN == 0
  DEFU (char);
#else
  DEF (char);
#endif
  DEF (short); DEF (int); DEF (long); DEF (long long);
  DEFU (unsigned short); DEFU (unsigned int); DEFU (unsigned long); DEFU (unsigned long long);
  DEF (float); DEF (double); DEF (long double);
#undef DEFU
#undef DEF
#undef DEF2
}

#endif // !MATH_ABS_HPP_INCLUDED
