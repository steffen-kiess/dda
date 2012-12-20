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

#ifndef CORE_NUMERICEXCEPTION_HPP_INCLUDED
#define CORE_NUMERICEXCEPTION_HPP_INCLUDED

// Exceptions thrown by Core::CheckedInteger on overflow
//
// All Exceptions inherit from Core::NumericException

#include <Core/Type.hpp>
#include <Core/Exception.hpp>
#include <Core/Assert.hpp>

#include <limits>
#include <ostream>
#include <sstream>
#include <typeinfo>

#include <stdint.h>
#include <limits.h>

namespace Core {
  namespace CheckedIntegerOperations {
    enum Operation {
      add, sub, mul, div, rem
    };
  }
  typedef CheckedIntegerOperations::Operation CheckedIntegerOperationsType;

  namespace Intern {
    template <typename T> inline std::string intToString (T a) {
      std::stringstream str;
      if (std::numeric_limits<T>::is_signed)
        str << (int64_t) a;
      else
        str << (uint64_t) a;
      return str.str ();
    }
  }

  class NumericException : public Exception {
  };

  class ConversionOverflowException : public virtual NumericException {
  };

  class BinaryOperationNumericException : public virtual NumericException {
    CheckedIntegerOperationsType _operation;

  public:
    BinaryOperationNumericException (CheckedIntegerOperationsType operation) : _operation (operation) {
    }

    static const char* operationToString (CheckedIntegerOperationsType operation) {
      switch (operation) {
      case CheckedIntegerOperations::add: return "+";
      case CheckedIntegerOperations::sub: return "-";
      case CheckedIntegerOperations::mul: return "*";
      case CheckedIntegerOperations::div: return "/";
      case CheckedIntegerOperations::rem: return "%";
      }
      ABORT ();
    }

    CheckedIntegerOperationsType operationCode () const {
      return _operation;
    }

    const char* operation () const {
      return operationToString (operationCode ());
    }
  };

  class BinaryOperationOverflowException : public virtual BinaryOperationNumericException {
  public:
    BinaryOperationOverflowException (CheckedIntegerOperationsType operation) : BinaryOperationNumericException (operation) {
    }
  };

  class DivisionByZeroException : public virtual BinaryOperationNumericException {
  public:
    DivisionByZeroException (CheckedIntegerOperationsType operation) : BinaryOperationNumericException (operation) {
    }
  };

  template <typename TargetType> class TargetTypedNumericException : public virtual NumericException {
  public:
    static bool isSigned () {
      return std::numeric_limits<TargetType>::is_signed;
    }

    static TargetType typeMin () {
      return std::numeric_limits<TargetType>::min ();
    }

    static TargetType typeMax () {
      return std::numeric_limits<TargetType>::max ();
    }

    static std::string targetTypeInfo () {
      std::stringstream str;

      str << "Type `" << Type::getName<TargetType> () << "' is " << (isSigned () ? "signed" : "unsigned") << ", min is " << Intern::intToString (typeMin ()) << ", max is " << Intern::intToString (typeMax ());
      
      return str.str ();
    }
  };

  template <typename T> class TypedBinaryOperationNumericException : public virtual TargetTypedNumericException<T>, public virtual BinaryOperationNumericException {
    T _a;
    T _b;

  public:
    TypedBinaryOperationNumericException (CheckedIntegerOperationsType operation, T a, T b) : BinaryOperationNumericException (operation), _a (a), _b (b) {
    }

    T a () const {
      return _a;
    }

    T b () const {
      return _b;
    }
  };

  template <typename T> class TypedBinaryOperationOverflowException : public virtual TypedBinaryOperationNumericException<T>, public virtual BinaryOperationOverflowException {
  public:
    TypedBinaryOperationOverflowException (CheckedIntegerOperationsType operation, T a, T b) : BinaryOperationNumericException (operation), TypedBinaryOperationNumericException<T> (operation, a, b), BinaryOperationOverflowException (operation) {
    }

    virtual std::string message () const {
      std::stringstream str;

      str << "Overflow for " << Intern::intToString (TypedBinaryOperationNumericException<T>::a ()) << " " << TypedBinaryOperationNumericException<T>::operation () << " " << Intern::intToString (TypedBinaryOperationNumericException<T>::b ()) << ". " << TargetTypedNumericException<T>::targetTypeInfo ();
      
      return str.str ();
    }
  };

  template <typename From, typename To>
  class TypedConversionOverflowException : public virtual TargetTypedNumericException<To>, public virtual ConversionOverflowException {
    From _value;

  public:
    TypedConversionOverflowException (From value) : _value (value) {
    }

    From value () const {
      return _value;
    }

    virtual std::string message () const {
      std::stringstream str;
      typedef std::numeric_limits<To> target;
      str << "Error converting from " << Type::getName<From> () << " to " << Type::getName<To> () << ": " << Intern::intToString (value ()) << " is not in [" << Intern::intToString (target::min ()) << ";" << Intern::intToString (target::max ()) << "]";
      return str.str ();
    }
  };

  template <typename T> class TypedDivisionByZeroException : public virtual TargetTypedNumericException<T>, public virtual DivisionByZeroException {
    T _dividend;

  public:
    TypedDivisionByZeroException (CheckedIntegerOperationsType operation, T dividend) : BinaryOperationNumericException (operation), DivisionByZeroException (operation), _dividend (dividend) {
    }

    T dividend () const {
      return _dividend;
    }

    virtual std::string message () const {
      std::stringstream str;
      str << "Error: Division by zero in " << Intern::intToString (dividend ()) << " " << operation () << " 0 " << TargetTypedNumericException<T>::targetTypeInfo ();
      return str.str ();
    }
  };
}

#endif // !CORE_NUMERICEXCEPTION_HPP_INCLUDED
