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

// Test Core::CheckedInteger and Core::checked_cast<>()

#include <Core/Util.hpp>

#if !defined (__GNUC__) || defined (__clang__) || GCC_VERSION_IS_ATLEAST (4, 6)
#if __has_warning ("-Wunused-but-set-variable") || GCC_VERSION_IS_ATLEAST (4, 6)
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#else
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#endif

#include <Core/CheckedIntegerAlias.hpp>
#include <Core/Time.hpp>
#include <Core/OStream.hpp>

int main () {
  try {
    Core::CheckedInteger<uint32_t> x = 5000000000;
    ABORT ();
  } catch (Core::ConversionOverflowException& e) {
  }

  try {
    Core::CheckedInteger<int32_t> y = 3000000000u;
    ABORT ();
  } catch (Core::ConversionOverflowException& e) {
  }

  Core::CheckedInteger<uint64_t> y1 = 5000000000ul;
  Core::CheckedInteger<int64_t> y2 = 5000000000ul;

  try {
    Core::CheckedInteger<int32_t> y = 3000000000ul;
    ABORT ();
  } catch (Core::ConversionOverflowException& e) {
  }

  cuint32_t v = 0;
  v /= cuint32_t::maxValue;

  cint32_t i = cint32_t::minValue;
  try {
    i /= -1;
    ABORT ();
  } catch (Core::BinaryOperationOverflowException& e) {
    ASSERT (e.operationCode () == Core::CheckedIntegerOperations::div);
  }
  
  try {
    i /= 0;
    ABORT ();
  } catch (Core::DivisionByZeroException& e) {
    ASSERT (e.operationCode () == Core::CheckedIntegerOperations::div);
  }
  
  try {
    v /= 0;
    ABORT ();
  } catch (Core::DivisionByZeroException& e) {
    ASSERT (e.operationCode () == Core::CheckedIntegerOperations::div);
  }

  cint64_t a1 = -3;
  cuint64_t a2 = 4;
  if (a1 > a2)
    ABORT ();

  cint64_t b1 = -1;
  cuint64_t b2 = cuint64_t::maxValue;
  if (b1 == b2)
    ABORT ();

  cint64_t c = 6;
  try {
    c %= 0;
  } catch (Core::DivisionByZeroException& e) {
    ASSERT (e.operationCode () == Core::CheckedIntegerOperations::rem);
  }

  Core::TimeSpan t1 = Core::getCurrentTime ();
  Core::TimeSpan t2 = Core::getCpuTime ();
  Core::TimeSpan t3 = Core::getCpuSystemTime ();
  Core::TimeSpan t4 = Core::getCpuUserTime ();

  Core::checked_cast<int32_t> ((uint64_t) 1000000000u);
  try {
    Core::checked_cast<int32_t> ((uint32_t) 4000000000u);
    ABORT ();
  } catch (Core::TypedConversionOverflowException<uint32_t, int32_t>& e) {
  }
  try {
    Core::checked_cast<int32_t> ((uint64_t) 4000000000u);
    ABORT ();
  } catch (Core::TypedConversionOverflowException<uint64_t, int32_t>& e) {
  }
  try {
    Core::checked_cast<uint64_t> (-3);
    ABORT ();
  } catch (Core::ConversionOverflowException& e) {
  }
  try {
    Core::checked_cast<uint8_t> (-3000000);
    ABORT ();
  } catch (Core::ConversionOverflowException& e) {
  }
  try {
    Core::checked_cast<uint32_t> ((uint64_t) 40000000000u);
    ABORT ();
  } catch (Core::TypedConversionOverflowException<uint64_t, uint32_t>& e) {
  }

  Core::checked_cast<int32_t> ((uint64_t) 1000000000u);
  try {
    Core::checked_cast<uint32_t> ((uint64_t) 40000000000u);
    ABORT ();
  } catch (Core::TypedConversionOverflowException<uint64_t, uint32_t>& e) {
  }

  Core::checked_cast<cint32_t> ((uint64_t) 1000000000u);
  try {
    Core::checked_cast<cuint32_t> ((uint64_t) 40000000000u);
    ABORT ();
  } catch (Core::TypedConversionOverflowException<uint64_t, uint32_t>& e) {
  }

  Core::checked_cast<int32_t> ((cuint64_t) (uint64_t) 1000000000u);
  try {
    Core::checked_cast<uint32_t> ((cuint64_t) (uint64_t) 40000000000u);
    ABORT ();
  } catch (Core::TypedConversionOverflowException<uint64_t, uint32_t>& e) {
  }

  Core::checked_cast<cint32_t> ((cuint64_t) (uint64_t) 1000000000u);
  try {
    Core::checked_cast<cuint32_t> ((cuint64_t) (uint64_t) 40000000000u);
    ABORT ();
  } catch (Core::TypedConversionOverflowException<uint64_t, uint32_t>& e) {
  }

  return 0;
}
