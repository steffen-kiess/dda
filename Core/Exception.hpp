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

#ifndef CORE_EXCEPTION_HPP_INCLUDED
#define CORE_EXCEPTION_HPP_INCLUDED

// Core::Exception is a base class for exceptions
//
// When a Core::Exception instance is created a stacktrace is generated. This
// stacktrace will be shown by .toString() or .what().
//
// Subclasses have to overwrite the message() method.

#include <exception>
#include <vector>
#include <string>
#include <ostream>

#include <stdint.h>

namespace Core {
  // Used for exceptions in the exception handling code
  class SimpleStdException : public std::exception {
    std::string descr_;

  public:
    SimpleStdException (std::string descr);
    ~SimpleStdException () throw ();

    virtual const char* what () const throw ();
  };

  class StackFrame;

  class InlineStackFrame {
    friend class StackFrame;

    std::string _method;
    std::string _sourceFile;
    uint64_t _lineNumber;

    InlineStackFrame (const std::string& method, const std::string& sourceFile, uint64_t lineNumber);

  public:
    const std::string& method () const {
      return _method;
    }

    const std::string& sourceFile () const {
      return _sourceFile;
    }

    uint64_t lineNumber () const {
      return _lineNumber;
    }

    std::string toString () const;
  };

  class StackFrame {
    friend class InlineStackFrame;

    void* _ptr;

    mutable bool _isResolved;
    mutable bool _hasSharedObject;
    mutable std::string _sharedObjectName;
    mutable void* _sharedObjectBase;
    mutable bool _hasSymbol;
    mutable std::string _symbolName;
    mutable void* _symbolAddr;
    void doResolve () const;

    mutable bool _hasAddr2line;
    mutable std::vector<InlineStackFrame> _inlineStackFrames;
    void doAddr2line () const;

  public:
    StackFrame (void* ptr);

    void* ptr () const {
      return _ptr;
    }

    void resolve () const {
      if (!_isResolved)
        doResolve ();
    }

    bool hasSharedObject () const {
      resolve ();
      return _hasSharedObject;
    }

    const std::string& sharedObjectName () const {
      resolve ();
      if (!_hasSharedObject)
        throw "!_hasSharedObject";
      return _sharedObjectName;
    }

    void* sharedObjectBase () const {
      resolve ();
      if (!_hasSharedObject)
        throw "!_hasSharedObject";
      return _sharedObjectBase;
    }

    size_t sharedObjectOffset () const {
      return (char*) ptr () - (char*) sharedObjectBase ();
    }

    bool hasSymbol () const {
      resolve ();
      return _hasSymbol;
    }

    const std::string& symbolName () const {
      resolve ();
      if (!_hasSymbol)
        throw "!_hasSymbol";
      return _symbolName;
    }

    void* symbolAddr () const {
      resolve ();
      if (!_hasSymbol)
        throw "!_hasSymbol";
      return _symbolAddr;
    }

    size_t symbolOffset () const {
      return (char*) ptr () - (char*) symbolAddr ();
    }

    const std::vector<InlineStackFrame>& inlineStackFrames () const {
      if (!_hasAddr2line)
        doAddr2line ();
      return _inlineStackFrames;
    }

    std::string toString (int* i = NULL, size_t* addrSize = NULL, size_t* symbSize = NULL, size_t* locSize = NULL) const;
  };

  class StackTrace {
    std::vector<StackFrame> _frames;
  public:
    StackTrace () {}

    StackTrace (const std::vector<StackFrame>& frames) : _frames (frames) {}

    struct CreateFromCurrentThread_t { };
    static const CreateFromCurrentThread_t createFromCurrentThread;
    StackTrace (CreateFromCurrentThread_t ignore);

    const std::vector<StackFrame>& frames () const {
      return _frames;
    }

    std::string toString () const;
  };

  class Exception : public std::exception {
    StackTrace _stackTrace;

    mutable std::string whatValue;
    mutable bool whatValueComputed;

  public:
    Exception () : _stackTrace (StackTrace::createFromCurrentThread), whatValueComputed (false) {}

    const StackTrace& stackTrace () const {
      return _stackTrace;
    }

    virtual ~Exception () throw ();

    virtual std::string message () const = 0;

    void writeTo (std::ostream& stream) const throw ();
    std::string toString () const throw ();
    virtual const char* what () const throw ();
  };

}

#endif // !CORE_EXCEPTION_HPP_INCLUDED
