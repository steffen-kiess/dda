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

#ifndef CORE_ASSERT_HPP_INCLUDED
#define CORE_ASSERT_HPP_INCLUDED

// ASSERT(x) and ABORT() macro
//
// These assertions are always enabled.
//
// The versions with _MSG accept a string as second argument. This argument
// will only be evaluated if the assertion fails.

#include <Core/Util.hpp>

#include <string>

namespace Core {
  namespace Intern {
    NORETURN assertionFailure (const char* file, int line, const char* condition, const std::string& additionalMessage = "");
  }

#define ASSERT_STR(condition, conditionStr) do {                        \
    if (!(condition)) {                                                 \
      ::Core::Intern::assertionFailure (__FILE__, __LINE__, conditionStr); \
    }                                                                   \
  } while (0)
#define ASSERT(condition) ASSERT_STR (condition, #condition)

#define ASSERT_STR_MSG(condition, conditionStr, additionalMessage) do { \
    if (!(condition)) {                                                 \
      ::Core::Intern::assertionFailure (__FILE__, __LINE__, conditionStr, additionalMessage); \
    }                                                                   \
  } while (0)
#define ASSERT_MSG(condition, additionalMessage) ASSERT_STR_MSG (condition, #condition, additionalMessage)

#define ABORT() do {                                                    \
    ::Core::Intern::assertionFailure (__FILE__, __LINE__, NULL);     \
  } while (0)

#define ABORT_MSG(additionalMessage) do {                               \
    ::Core::Intern::assertionFailure (__FILE__, __LINE__, NULL, additionalMessage); \
  } while (0)

}

#endif // !CORE_ASSERT_HPP_INCLUDED
