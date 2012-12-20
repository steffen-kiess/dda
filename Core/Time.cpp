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

#include "Time.hpp"

#include <cstdio>

#include <Core/Exception.hpp>

#include <Core/Util.hpp>
#include <Core/Assert.hpp>
#include <Core/Error.hpp>
#include <Core/WindowsError.hpp>

#if OS_UNIX
#include <cstddef>
#include <sys/time.h>
#include <sys/resource.h>
#elif OS_WIN
#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef min
#undef max
#else
#error "Not implemented for this OS"
#endif

namespace Core {
#if OS_UNIX
  static inline TimeSpan toTimeSpan (struct timeval time) {
    return TimeSpan (int64_t (time.tv_sec) * 1000000 + int64_t (time.tv_usec));
  }
#endif
#if OS_WIN
  static inline TimeSpan toTimeSpan (FILETIME time) {
    uint64_t high = (uint32_t) time.dwHighDateTime;
    uint64_t low = (uint64_t) time.dwLowDateTime;
    uint64_t value = (high << 32) | low;
    if (value + 5 > value)
      value += 5;
    value /= 10;
    return TimeSpan (value);
  }
#endif

  TimeSpan getCurrentTime () {
#if OS_UNIX
    struct timeval time;
    gettimeofday(&time, NULL);
    return toTimeSpan (time);
#elif OS_WIN
    LARGE_INTEGER freq;
    Core::WindowsError::check ("QueryPerformanceFrequency", QueryPerformanceFrequency (&freq));

    LARGE_INTEGER time;
    Core::WindowsError::check ("QueryPerformanceCounter", QueryPerformanceCounter (&time));

    return TimeSpan::fromSeconds (double (time.QuadPart) / double (freq.QuadPart));
#else
#error "Not implemented for this OS"
#endif
  }

  TimeSpan getCpuTime () {
#if OS_UNIX
    struct rusage usage;
    Core::Error::check ("getrusage", getrusage (RUSAGE_SELF, &usage));
    return toTimeSpan (usage.ru_utime) + toTimeSpan (usage.ru_stime);
#elif OS_WIN
    FILETIME creation, exit, kernel, user;
    Core::WindowsError::check ("GetThreadTimes", GetThreadTimes (GetCurrentThread (), &creation, &exit, &kernel, &user));
    return toTimeSpan (kernel) + toTimeSpan (user);
#else
#error "Not implemented for this OS"
#endif
  }

  TimeSpan getCpuSystemTime () {
#if OS_UNIX
    struct rusage usage;
    Core::Error::check ("getrusage", getrusage (RUSAGE_SELF, &usage));
    return toTimeSpan (usage.ru_stime);
#elif OS_WIN
    FILETIME creation, exit, kernel, user;
    Core::WindowsError::check ("GetThreadTimes", GetThreadTimes (GetCurrentThread (), &creation, &exit, &kernel, &user));
    return toTimeSpan (kernel);
#else
#error "Not implemented for this OS"
#endif
  }

  TimeSpan getCpuUserTime () {
#if OS_UNIX
    struct rusage usage;
    Core::Error::check ("getrusage", getrusage (RUSAGE_SELF, &usage));
    return toTimeSpan (usage.ru_utime);
#elif OS_WIN
    FILETIME creation, exit, kernel, user;
    Core::WindowsError::check ("GetThreadTimes", GetThreadTimes (GetCurrentThread (), &creation, &exit, &kernel, &user));
    return toTimeSpan (user);
#else
#error "Not implemented for this OS"
#endif
  }
}
