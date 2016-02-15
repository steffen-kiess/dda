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

#ifndef CORE_NUMERICCHECKEDINTEGEREXCEPTION_HPP_INCLUDED
#define CORE_NUMERICCHECKEDINTEGEREXCEPTION_HPP_INCLUDED

// Exceptions thrown by Core::CheckedInteger on overflow
//
// All Exceptions inherit from Core::NumericException

#include <Core/NumericException.hpp>

namespace Core {
  namespace CheckedIntegerOperations {
    enum Operation {
      add, sub, mul, div, rem
    };
  }
  typedef CheckedIntegerOperations::Operation CheckedIntegerOperationsType;

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

#endif // !CORE_NUMERICCHECKEDINTEGEREXCEPTION_HPP_INCLUDED
