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

#include "Assert.hpp"

#include <Core/Exception.hpp>

#include <string.h>
#include <sstream>

namespace Core {
  namespace Intern {
    class AssertionFailureBase : public Exception {
      std::string _file;
      int _line;
      std::string _additionalMessage;

    public:
      AssertionFailureBase (std::string file, int line, std::string additionalMessage) : _file (file), _line (line), _additionalMessage (additionalMessage) {}
      ~AssertionFailureBase () throw () {}

      const std::string& file () const {
        return _file;
      }

      int line () const {
        return _line;
      }

      const std::string& additionalMessage () const {
        return _additionalMessage;
      }
    };

    class AssertionFailure : public AssertionFailureBase {
      std::string _condition;

    public:
      AssertionFailure (std::string file, int line, std::string condition, std::string additionalMessage) : AssertionFailureBase (file, line, additionalMessage), _condition (condition) {}
      ~AssertionFailure () throw () {}

      const std::string& condition () const {
        return _condition;
      }

      virtual std::string message () const {
        std::stringstream str;
        str << "Assertion `" << condition () << "' failed in " << file () << ":" << line ();
        if (additionalMessage () != "")
          str << ": `" << additionalMessage () << "'";
        return str.str ();
      }
    };

    class AbortCalled : public AssertionFailureBase {
    public:
      AbortCalled (std::string file, int line, std::string additionalMessage) : AssertionFailureBase (file, line, additionalMessage) {}
      ~AbortCalled () throw () {}

      virtual std::string message () const {
        std::stringstream str;
        str << "Abort called in " << file () << ":" << line ();
        if (additionalMessage () != "")
          str << ": `" << additionalMessage () << "'";
        return str.str ();
      }
    };

    void assertionFailure (const char* file, int line, const char* condition, const std::string& additionalMessage) {
      if (condition)
        throw AssertionFailure (file, line, condition, additionalMessage);
      else
        throw AbortCalled (file, line, additionalMessage);
    }
  }
}
