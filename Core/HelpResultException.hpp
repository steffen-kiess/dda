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

#ifndef CORE_HELPRESULTEXCEPTION_HPP_INCLUDED
#define CORE_HELPRESULTEXCEPTION_HPP_INCLUDED

// Core::HelpResultException is thrown when "help" was passed to some parsing
// method.
// The exception contains a list of allowed values.
// Does not inherit from Core::Exception because a stacktrace probably is not
// needed.

#include <string>
#include <exception>

namespace Core {
  class HelpResultException : public std::exception {
    std::string info_;
    std::string what_;

  public:
    HelpResultException (const std::string& info);
    ~HelpResultException () throw ();

    const std::string& info () const { return info_; }

    virtual const char* what () const throw ();
  };
}

#endif // !CORE_HELPRESULTEXCEPTION_HPP_INCLUDED
