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

#ifndef CORE_UTIL_H_INCLUDED
#define CORE_UTIL_H_INCLUDED

// Various preprocessor macros for C/C++

// clang feature check macros (for non-clang compilers)
#ifndef __has_builtin
#  define __has_builtin(x) 0
#endif
#ifndef __has_feature
#  define __has_feature(x) 0
#endif
#ifndef __has_extension
#  define __has_extension __has_feature
#endif
#ifndef __has_attribute
#  define __has_attribute(x) 0
#endif
#ifndef __has_warning
#  define __has_warning(x) 0
#endif

// Evaluates to true iff __GNUC__ is defined and (__GNUC__, __GNUC_MINOR__) is at least (major, minor)
#define GCC_VERSION_IS_ATLEAST(major, minor) (defined (__GNUC__) && (__GNUC__ > (major) || (__GNUC__ == (major) && __GNUC_MINOR__ >= (minor))))

// For methods which should never be called
#if __has_attribute(error) || GCC_VERSION_IS_ATLEAST(4, 3)
#define ERROR_ATTRIBUTE(text) __attribute__ ((__error__ (text)))
#else
#define ERROR_ATTRIBUTE(text)
#endif

// "unused" attribute to disable warnings
#if __has_attribute(unused) || defined (__GNUC__)
#define UNUSED __attribute__ ((unused))
#else
#define UNUSED
#endif

// For methods which never return
#if __has_attribute(noreturn) || defined (__GNUC__)
#define NORETURN_ATTRIBUTE __attribute__ ((noreturn))
#else
#define NORETURN_ATTRIBUTE
#endif

#define NORETURN NORETURN_ATTRIBUTE void

#if defined(__POSIX__) || defined (__unix) || defined (__unix__)
#define OS_UNIX 1
#define OS_WIN 0
#elif defined(__APPLE__)
#define OS_UNIX 1
#define OS_WIN 0
#elif defined(_WIN32)
#define OS_UNIX 0
#define OS_WIN 1
#else
#error "None of __POSIX__, __unix, __unix__, __APPLE__ and _WIN32 is defined."
#endif

#endif // !CORE_UTIL_H_INCLUDED
