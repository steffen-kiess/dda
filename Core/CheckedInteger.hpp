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

#ifndef CORE_CHECKEDINTEGER_HPP_INCLUDED
#define CORE_CHECKEDINTEGER_HPP_INCLUDED

// Integer class with overflow checks
//
// Core::CheckedInteger<T> is a wrapper for integer types which will check
// for overflow on +, -, *, /, % and on conversions
//
// The type provides an operator() which will return the value as the underlying
// type (T)
//
// Core::checked_cast<T> (value) can be used to cast one integer type (builtin
// or Core::CheckedInteger<T>) to another (builtin or Core::CheckedInteger<T>)
// with a range check.

#include <Core/Util.hpp>
#include <Core/Exception.hpp>
#include <Core/Assert.hpp>
#include <Core/NumericException.hpp>

#include <boost/integer_traits.hpp>
#include <boost/integer.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/type_traits/make_unsigned.hpp>

namespace Core {
  namespace Intern {
    template <int i> struct my_int_t {
      typedef typename boost::int_t<i>::fast fast;
    };
    template <int i> struct my_uint_t {
      typedef typename boost::uint_t<i>::fast fast;
    };
    template <> struct my_int_t<64> { typedef int64_t fast; };
    template <> struct my_uint_t<64> { typedef uint64_t fast; };

    template <typename T, typename U> struct ConversionInfo {
      static const bool sourceInt = std::numeric_limits<U>::is_integer;
      static const bool sourceSigned = std::numeric_limits<U>::is_signed;
      static const U sourceMin = boost::integer_traits<U>::const_min;
      static const U sourceMax = boost::integer_traits<U>::const_max;
      static const int sourceDigits = std::numeric_limits<U>::digits;

      static const bool targetSigned = std::numeric_limits<T>::is_signed;
      static const T targetMin = boost::integer_traits<T>::const_min;
      static const T targetMax = boost::integer_traits<T>::const_max;
      static const int targetDigits = std::numeric_limits<T>::digits;

      static const bool signedEqual = (sourceSigned && targetSigned) || (!sourceSigned && !targetSigned);

      static const bool isWidening = sourceInt
        && (
            // (signedEqual && (targetMin <= sourceMin) && (targetMax >= sourceMax) && ((targetMin < sourceMin) || (targetMax > sourceMax)))
            (signedEqual && targetDigits > sourceDigits)
            || (targetSigned && !sourceSigned && (targetDigits >= sourceDigits + 1))
            );

      static const bool isWideningOrEqual = sourceInt
        && (
            // (signedEqual && (targetMin <= sourceMin) && (targetMax >= sourceMax) && ((targetMin < sourceMin) || (targetMax > sourceMax)))
            (signedEqual && targetDigits >= sourceDigits)
            || (targetSigned && !sourceSigned && (targetDigits >= sourceDigits + 1))
            );
    };

    template <typename T, CheckedIntegerOperationsType op> NORETURN overflow (T a, T b) {
      throw TypedBinaryOperationOverflowException<T> (op, a, b);
    }
    template <typename To, typename From> NORETURN overflow (From value) {
      throw TypedConversionOverflowException<From, To> (value);
    }
    template <typename T, CheckedIntegerOperationsType op> NORETURN divByZero (T dividend) {
      throw TypedDivisionByZeroException<T> (op, dividend);
    }

    template <typename T, typename U> struct ConverterSameSign {
      static inline T convert (U v) {
        typedef std::numeric_limits<T> target;

        if (v < target::min () || v > target::max ()) {
          overflow<T, U> (v);
        }
        return (T) v;
      }
    };

    template <typename T, typename U> struct ConverterSU {
      static inline T convert (U v) {
        typedef std::numeric_limits<T> target;

        if (v < 0 || (typename boost::make_unsigned<U>::type) v > target::max ()) {
          overflow<T, U> (v);
        }

        return (T) v;
      }
    };

    template <typename T, typename U> struct ConverterUS {
      static inline T convert (U v) {
        typedef std::numeric_limits<T> target;

        if (v > (typename boost::make_unsigned<T>::type) target::max ()) {
          overflow<T, U> (v);
        }

        return (T) v;
      }
    };

    // checked_cast int => cint
    template <typename To, typename From> struct CheckedConverter {
      BOOST_STATIC_ASSERT (std::numeric_limits<To>::is_integer);
      BOOST_STATIC_ASSERT (std::numeric_limits<From>::is_integer);

      typedef ConversionInfo<To, From> Info;
      typedef typename boost::mpl::if_c<Info::signedEqual, ConverterSameSign<To, From>, typename boost::mpl::if_c<Info::sourceSigned, ConverterSU<To, From> , ConverterUS<To, From> >::type >::type Conv;

      static inline To convert (From value) {
        return Conv::convert (value);
      }
    };
  }

  template <typename To, typename From>
  inline To checked_cast (From value) {
    return Intern::CheckedConverter<To, From>::convert (value);
  }

  template <typename T> class CheckedInteger {
    BOOST_STATIC_ASSERT (std::numeric_limits<T>::is_integer);
    BOOST_STATIC_ASSERT (std::numeric_limits<T>::digits != 0);

    class PrivateType {
      friend class CheckedInteger;
      PrivateType () {}
    };

  public:
    static const T minValue = boost::integer_traits<T>::const_min;
    static const T maxValue = boost::integer_traits<T>::const_max;
    static const bool isSigned = std::numeric_limits<T>::is_signed;

  private:
    BOOST_STATIC_ASSERT (isSigned || minValue == 0);
    BOOST_STATIC_ASSERT (!isSigned || -(minValue + 1) == maxValue);

    T _value;

    static inline T checked_add (T a, T b) {
      typedef typename Intern::my_int_t<std::numeric_limits<T>::digits + 2>::fast LargerType;
      LargerType sum = (LargerType) a + (LargerType) b;
      if (sum < std::numeric_limits<T>::min () || sum > std::numeric_limits<T>::max ())
        Intern::overflow<T, CheckedIntegerOperations::add> (a, b);
      return sum;
    }
    static inline T checked_sub (T a, T b) {
      typedef typename Intern::my_int_t<std::numeric_limits<T>::digits + 2>::fast LargerType;
      LargerType sum = (LargerType) a - (LargerType) b;
      if (sum < std::numeric_limits<T>::min () || sum > std::numeric_limits<T>::max ())
        Intern::overflow<T, CheckedIntegerOperations::sub> (a, b);
      return sum;
    }
    static inline T checked_mul (T a, T b) {
      typedef typename boost::mpl::if_c<std::numeric_limits<T>::is_signed, typename Intern::my_int_t<std::numeric_limits<T>::is_signed ? 2 * std::numeric_limits<T>::digits + 2 : 8>::fast, typename Intern::my_uint_t<std::numeric_limits<T>::is_signed ? 8 : 2 * std::numeric_limits<T>::digits>::fast>::type LargerType;
      LargerType res = (LargerType) a * (LargerType) b;
      if (res < std::numeric_limits<T>::min () || res > std::numeric_limits<T>::max ())
        Intern::overflow<T, CheckedIntegerOperations::mul> (a, b);
      return static_cast<T> (res);
    }
    static inline T checked_div (T a, T b) {
      if (!b)
        Intern::divByZero<T, CheckedIntegerOperations::div> (a);
      if (isSigned)
        if (a == minValue && b == (isSigned ? -1 : 0))
          Intern::overflow<T, CheckedIntegerOperations::div> (a, b);
      return a / b;
    }
    static inline T checked_rem (T a, T b) {
      if (!b)
        Intern::divByZero<T, CheckedIntegerOperations::rem> (a);
      return a % b;
    }

  public:
    template <typename U>
    CheckedInteger (U value, UNUSED typename boost::enable_if_c<std::numeric_limits<U>::is_integer, PrivateType>::type dummy = PrivateType ()) {
      _value = Intern::CheckedConverter<T, U>::convert (value);
    }

    template <typename U>
    explicit CheckedInteger (CheckedInteger<U> v, UNUSED typename boost::enable_if_c<std::numeric_limits<U>::is_integer && !Intern::ConversionInfo<T, U>::isWidening, PrivateType>::type dummy = PrivateType ()) {
      *this = v.value ();
    }

    template <typename U>
    CheckedInteger (CheckedInteger<U> v, UNUSED typename boost::enable_if_c<std::numeric_limits<U>::is_integer && Intern::ConversionInfo<T, U>::isWidening, PrivateType>::type dummy = PrivateType ()) {
      _value = v.value ();
    }

    /*
#if HAVE_CXX11
    explicit operator T () const {
      return _value;
    }
#endif
    */

    T value () const {
      return _value;
    }

    T operator () () const {
      return _value;
    }

    CheckedInteger operator+ (CheckedInteger o) const {
      return CheckedInteger (checked_add (value (), o.value ()));
    }

    CheckedInteger operator- (CheckedInteger o) const {
      return CheckedInteger (checked_sub (value (), o.value ()));
    }

    CheckedInteger& operator+= (CheckedInteger o) {
      _value = checked_add (value (), o.value ());
      return *this;
    }

    CheckedInteger& operator-= (CheckedInteger o) {
      _value = checked_sub (value (), o.value ());
      return *this;
    }

    CheckedInteger& operator++ () {
      _value = checked_add (value (), 1);
      return *this;
    }

    CheckedInteger operator++ (UNUSED int unused) {
      CheckedInteger ret = *this;
      _value = checked_add (value (), 1);
      return ret;
    }

    CheckedInteger& operator-- () {
      _value = checked_sub (value (), 1);
      return *this;
    }

    CheckedInteger operator-- (UNUSED int unused) {
      CheckedInteger ret = *this;
      _value = checked_sub (value (), 1);
      return ret;
    }

    CheckedInteger operator* (CheckedInteger o) const {
      return CheckedInteger (checked_mul (value (), o.value ()));
    }

    CheckedInteger operator/ (CheckedInteger o) const {
      return CheckedInteger (checked_div (value (), o.value ()));
    }

    CheckedInteger operator% (CheckedInteger o) const {
      return CheckedInteger (checked_rem (value (), o.value ()));
    }

    CheckedInteger& operator*= (CheckedInteger o) {
      _value = checked_mul (value (), o.value ());
      return *this;
    }

    CheckedInteger& operator/= (CheckedInteger o) {
      _value = checked_div (value (), o.value ());
      return *this;
    }

    CheckedInteger& operator%= (CheckedInteger o) {
      _value = checked_rem (value (), o.value ());
      return *this;
    }

    /*
#define FORWARD_BOOL_OP(op)                     \
    bool operator op (CheckedInteger o) const { \
      return value () op o.value ();            \
    }
    FORWARD_BOOL_OP (<) FORWARD_BOOL_OP (<=)
    FORWARD_BOOL_OP (>) FORWARD_BOOL_OP (>=)
    FORWARD_BOOL_OP (==) FORWARD_BOOL_OP (!=)
#undef FORWARD_BOOL_OP
    */
  };

  namespace Intern {
    // checked_cast int => cint
    template <typename To, typename From> struct CheckedConverter<CheckedInteger<To>, From> {
      static inline To convert (From value) {
        return To (CheckedConverter<To, From>::convert (value));
      }
    };
    // checked_cast cint => int
    template <typename To, typename From> struct CheckedConverter<To, CheckedInteger<From> > {
      static inline To convert (CheckedInteger<From> value) {
        return CheckedConverter<To, From>::convert (value ());
      }
    };
    // checked_cast cint => cint
    template <typename To, typename From> struct CheckedConverter<CheckedInteger<To>, CheckedInteger<From> > {
      static inline To convert (CheckedInteger<From> value) {
        return To (CheckedConverter<To, From>::convert (value ()));
      }
    };

    class CI_PrivateType {
    };

    /*signed == unsigned*/ template <typename T, typename U> inline bool CI_compare_equals (T v1, U v2, UNUSED typename boost::enable_if_c<std::numeric_limits<T>::is_integer && std::numeric_limits<U>::is_integer && std::numeric_limits<T>::is_signed && !std::numeric_limits<U>::is_signed && (std::numeric_limits<T>::digits < std::numeric_limits<U>::digits), CI_PrivateType>::type dummy = CI_PrivateType ()) {
      return v1 >= 0 && static_cast<U> (v1) == v2;
    }
    /*unsigned == signed*/ template <typename T, typename U> inline bool CI_compare_equals (T v1, U v2, UNUSED typename boost::enable_if_c<std::numeric_limits<T>::is_integer && std::numeric_limits<U>::is_integer && !std::numeric_limits<T>::is_signed && std::numeric_limits<U>::is_signed && (std::numeric_limits<T>::digits > std::numeric_limits<U>::digits), CI_PrivateType>::type dummy = CI_PrivateType ()) {
      return v2 >= 0 && v1 == static_cast<T> (v2);
    }

    /*signed < unsigned*/ template <typename T, typename U> inline bool CI_compare_smaller (T v1, U v2, UNUSED typename boost::enable_if_c<std::numeric_limits<T>::is_integer && std::numeric_limits<U>::is_integer && std::numeric_limits<T>::is_signed && !std::numeric_limits<U>::is_signed && (std::numeric_limits<T>::digits < std::numeric_limits<U>::digits), CI_PrivateType>::type dummy = CI_PrivateType ()) {
      return v1 < 0 || static_cast<U> (v1) < v2;
    }
    /*unsigned < signed*/ template <typename T, typename U> inline bool CI_compare_smaller (T v1, U v2, UNUSED typename boost::enable_if_c<std::numeric_limits<T>::is_integer && std::numeric_limits<U>::is_integer && !std::numeric_limits<T>::is_signed && std::numeric_limits<U>::is_signed && (std::numeric_limits<T>::digits > std::numeric_limits<U>::digits), CI_PrivateType>::type dummy = CI_PrivateType ()) {
      return v2 >= 0 && v1 < static_cast<T> (v2);
    }

    /*signed > unsigned*/ template <typename T, typename U> inline bool CI_compare_greater (T v1, U v2, UNUSED typename boost::enable_if_c<std::numeric_limits<T>::is_integer && std::numeric_limits<U>::is_integer && std::numeric_limits<T>::is_signed && !std::numeric_limits<U>::is_signed && (std::numeric_limits<T>::digits < std::numeric_limits<U>::digits), CI_PrivateType>::type dummy = CI_PrivateType ()) {
      return v1 >= 0 && static_cast<U> (v1) > v2;
    }
    /*unsigned > signed*/ template <typename T, typename U> inline bool CI_compare_greater (T v1, U v2, UNUSED typename boost::enable_if_c<std::numeric_limits<T>::is_integer && std::numeric_limits<U>::is_integer && !std::numeric_limits<T>::is_signed && std::numeric_limits<U>::is_signed && (std::numeric_limits<T>::digits > std::numeric_limits<U>::digits), CI_PrivateType>::type dummy = CI_PrivateType ()) {
      return v2 < 0 || v1 > static_cast<T> (v2);
    }
  }
#define FORWARD_BOOL_OP(n, op, rop)                                     \
  namespace Intern {                                                    \
    template <typename T, typename U> inline bool CI_compare_##n (T v1, U v2, UNUSED typename boost::enable_if_c<std::numeric_limits<T>::is_integer && std::numeric_limits<U>::is_integer && Intern::ConversionInfo<T, U>::isWideningOrEqual, CI_PrivateType>::type dummy = CI_PrivateType ()) { \
      T v2x = v2;                                                       \
      return v1 op v2x;                                                 \
    }                                                                   \
    template <typename T, typename U> inline bool CI_compare_##n (T v1, U v2, UNUSED typename boost::enable_if_c<std::numeric_limits<T>::is_integer && std::numeric_limits<U>::is_integer && Intern::ConversionInfo<U, T>::isWidening, CI_PrivateType>::type dummy = CI_PrivateType ()) { \
      U v1x = v1;                                                       \
      return v1x op v2;                                                 \
    }                                                                   \
  }                                                                     \
  template <typename T, typename U> inline bool operator op (CheckedInteger<T> v1, CheckedInteger<U> v2) { \
    return Intern::CI_compare_##n<T, U> (v1.value (), v2.value ());     \
  }                                                                     \
  template <typename T, typename U> inline bool operator rop (CheckedInteger<T> v1, CheckedInteger<U> v2) { \
    return !Intern::CI_compare_##n<T, U> (v1.value (), v2.value ());    \
  }                                                                     \
  template <typename T, typename U> inline bool operator op (CheckedInteger<T> v1, U v2) { \
    return Intern::CI_compare_##n<T, U> (v1.value (), v2);              \
  }                                                                     \
  template <typename T, typename U> inline bool operator rop (CheckedInteger<T> v1, U v2) { \
    return !Intern::CI_compare_##n<T, U> (v1.value (), v2);             \
  }                                                                     \
  template <typename T, typename U> inline bool operator op (T v1, CheckedInteger<U> v2) { \
    return Intern::CI_compare_##n<T, U> (v1, v2.value ());              \
  }                                                                     \
  template <typename T, typename U> inline bool operator rop (T v1, CheckedInteger<U> v2) { \
    return !Intern::CI_compare_##n<T, U> (v1, v2.value ());             \
  }

  FORWARD_BOOL_OP (smaller, <, >=)
  FORWARD_BOOL_OP (greater, >, <=)
  FORWARD_BOOL_OP (equals, ==, !=)
#undef FORWARD_BOOL_OP

  template <typename T> inline std::ostream& operator<< (std::ostream& stream, CheckedInteger<T> i) {
    stream << i.value ();
    return stream;
  }

#if defined (__amd64__) || defined (__i386__)

#define DEF_OP(ty, fc, op)                                              \
  template <> inline ty CheckedInteger<ty>::checked_##op (ty a, ty b) { \
    ty out;                                                             \
    uint8_t of;                                                         \
    asm (#op " %3, %0\n\t"                                              \
         #fc " %1\n\t"                                                  \
         : "=q" (out), "=q" (of)                                        \
         : "0" (a), "q" (b)                                             \
         : "cc"                                                         \
         );                                                             \
    if (of)                                                             \
      Intern::overflow<ty, CheckedIntegerOperations::op> (a, b);        \
    return out;                                                         \
  }

#define DEF_OPS(ty, fc) DEF_OP (ty, fc, add) DEF_OP (ty, fc, sub)

#if CHAR_MIN == 0
  DEF_OPS (char, setc)
#elif CHAR_MIN == SCHAR_MIN
  DEF_OPS (char, seto)
#else
#error "CHAR_MIN != 0 and CHAR_MIN != SCHAR_MIN"
#endif

  DEF_OPS (int8_t, seto) DEF_OPS (uint8_t, setc)
  DEF_OPS (int16_t, seto) DEF_OPS (uint16_t, setc)
  DEF_OPS (int32_t, seto) DEF_OPS (uint32_t, setc)
#if defined (__amd64__)
  DEF_OPS (int64_t, seto) DEF_OPS (uint64_t, setc)
#else
  /*
  template <> inline uint64_t CheckedInteger<uint64_t>::checked_add (uint64_t a, uint64_t b) {
    uint64_t sum = a + b;
    if (sum < a)
      Intern::overflow<uint64_t, CheckedIntegerOperations::add> (a, b);
    return sum;
  }
  template <> inline uint64_t CheckedInteger<uint64_t>::checked_sub (uint64_t a, uint64_t b) {
    uint64_t sum = a - b;
    if (sum > a)
      Intern::overflow<uint64_t, CheckedIntegerOperations::sub> (a, b);
    return sum;
  }
  */
#undef DEF_OP
#undef DEF_OPS
#define DEF_OPS(ty, fc) DEF_OP (ty, fc, add, adc) DEF_OP (ty, fc, sub, sbb)
  // http://gcc.gnu.org/onlinedocs/gcc/Machine-Constraints.html#Machine-Constraints
  // "of" must be a register acessible as "Xl", so use "q" constraint
  // out/a are put in ax:dx ("A")
  // // b is put in two registers out of (eax, ebx, ecx, edx, esi, edi, ebp, esp) ("R")
  // b can be "r/m32" or "imm32" according to the documentation of "ADD" in "Intel(R) 64 and IA-32 Architectures Software Developer's Manual, Volume 2A", which is "g" (http://gcc.gnu.org/onlinedocs/gcc/Simple-Constraints.html#Simple-Constraints)
  // http://gcc.gnu.org/onlinedocs/gcc/Modifiers.html#Modifiers
  // "out" is earlyclobber (eax is written before %4 is read which will cause
  //   problems if %4 is assigned to eax)
#define DEF_OP(ty, fc, op, opc)                                         \
  template <> inline ty CheckedInteger<ty>::checked_##op (ty a, ty b) { \
    ty out;                                                             \
    uint8_t of;                                                         \
    asm (#op " %3, %%eax\n\t"                                           \
         #opc " %4, %%edx\n\t"                                          \
         #fc " %1\n\t"                                                  \
         : "=&A" (out), "=q" (of)                                       \
           /*: "0" (a), "R" ((uint32_t) (uint64_t) b), "R" ((uint32_t) (((uint64_t) b) >> 32))*/ \
         : "0" (a), "g" ((uint32_t) (uint64_t) b), "g" ((uint32_t) (((uint64_t) b) >> 32)) \
         : "cc"                                                         \
         );                                                             \
    if (of)                                                             \
      Intern::overflow<ty, CheckedIntegerOperations::op> (a, b);        \
    return out;                                                         \
  }
  DEF_OPS (int64_t, seto) DEF_OPS (uint64_t, setc)
#endif

#undef DEF_OPS
#undef DEF_OP

#if defined (__amd64__)
  template <> inline uint64_t CheckedInteger<uint64_t>::checked_mul (uint64_t a, uint64_t b) {
    uint64_t out, hi;
    asm ("mul %3"
         : "=a" (out), "=d" (hi)
         : "a" (a), "q" (b)
         : "cc"
         );
    if (hi)
      Intern::overflow<uint64_t, CheckedIntegerOperations::mul> (a, b);
    return out;
  }

  template <> inline int64_t CheckedInteger<int64_t>::checked_mul (int64_t a, int64_t b) {
    int64_t out, hi;
    asm ("imul %3"
         : "=a" (out), "=d" (hi)
         : "a" (a), "q" (b)
         : "cc"
         );
    if (out < 0 ? (hi != -1) : (hi != 0))
      Intern::overflow<int64_t, CheckedIntegerOperations::mul> (a, b);
    return out;
  }
#endif
}

#endif

#endif // !CORE_CHECKEDINTEGER_HPP_INCLUDED
