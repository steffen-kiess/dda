/*
 * Copyright (c) 2010-2014 Steffen Kieß
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

#ifndef CORE_CHECKEDCAST_HPP_INCLUDED
#define CORE_CHECKEDCAST_HPP_INCLUDED

// Core::checked_cast<T> (value) can be used to cast one integer type (builtin
// or Core::CheckedInteger<T>) to another (builtin or Core::CheckedInteger<T>)
// with a range check.

#include <Core/Util.hpp>
#include <Core/Exception.hpp>
#include <Core/Assert.hpp>
#include <Core/NumericException.hpp>

#include <boost/integer_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/make_unsigned.hpp>

namespace Core {
  namespace Intern {
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

    template <typename To, typename From> NORETURN overflow (From value) {
      throw TypedConversionOverflowException<From, To> (value);
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
}

#endif // !CORE_CHECKEDCAST_HPP_INCLUDED
