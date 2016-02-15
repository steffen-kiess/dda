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

#ifndef CORE_TIMESPAN_HPP_INCLUDED
#define CORE_TIMESPAN_HPP_INCLUDED

// Core::TimeSpan represents the length of an interval of time.
//
// The length is stored as a signed 64-bit integer in microseconds.

#include <stdint.h>
#include <string>
#include <ostream>

namespace Core {
  class TimeSpan {
    int64_t value;

  public:
    explicit TimeSpan (int64_t value) : value (value) {
    }

    static TimeSpan fromSeconds (double value) {
      return TimeSpan (static_cast<int64_t> (value * 1000000.0));
    }

    int64_t getMicroseconds () const {
      return value;
    }

    double getSeconds () const {
      return double (getMicroseconds ()) / 1000000.0;
    }

    double getMilliseconds () const {
      return double (getMicroseconds ()) / 1000.0;
    }

    std::string toString (bool appendUnit = true) const;
    static TimeSpan parse (const std::string& str, bool appendUnit = true);

    TimeSpan operator +(TimeSpan o) {
      return TimeSpan (getMicroseconds () + o.getMicroseconds ());
    }

    TimeSpan operator -(TimeSpan o) {
      return TimeSpan (getMicroseconds () - o.getMicroseconds ());
    }

#define FORWARD_BOOL_OP(op)                                     \
    bool operator op (TimeSpan o) const {                       \
      return getMicroseconds () op o.getMicroseconds ();        \
    }
    FORWARD_BOOL_OP (<) FORWARD_BOOL_OP (<=)
    FORWARD_BOOL_OP (>) FORWARD_BOOL_OP (>=)
    FORWARD_BOOL_OP (==) FORWARD_BOOL_OP (!=)
#undef FORWARD_BOOL_OP
  };

  static inline TimeSpan operator *(TimeSpan t1, int s) {
    return TimeSpan (t1.getMicroseconds () * s);
  }

  static inline TimeSpan operator *(TimeSpan t1, double s) {
    return TimeSpan (static_cast<int64_t> (static_cast<double> (t1.getMicroseconds ()) * s));
  }

  static inline TimeSpan operator *(TimeSpan t1, float s) {
    return TimeSpan (static_cast<int64_t> (static_cast<double> (t1.getMicroseconds ()) * s));
  }

  static inline TimeSpan operator *(int s, TimeSpan t1) {
    return TimeSpan (t1.getMicroseconds () * s);
  }

  static inline TimeSpan operator *(double s, TimeSpan t1) {
    return TimeSpan (static_cast<int64_t> (static_cast<double> (t1.getMicroseconds ()) * s));
  }

  static inline TimeSpan operator *(float s, TimeSpan t1) {
    return TimeSpan (static_cast<int64_t> (static_cast<double> (t1.getMicroseconds ()) * s));
  }

  static inline TimeSpan operator /(TimeSpan t1, int s) {
    return TimeSpan (t1.getMicroseconds () / s);
  }

  static inline TimeSpan operator /(TimeSpan t1, double s) {
    return TimeSpan (static_cast<int64_t> (static_cast<double> (t1.getMicroseconds ()) / s));
  }

  static inline TimeSpan operator /(TimeSpan t1, float s) {
    return TimeSpan (static_cast<int64_t> (static_cast<double> (t1.getMicroseconds ()) / s));
  }

  static inline std::ostream& operator<< (std::ostream& stream, TimeSpan t) {
    stream << t.toString ();
    return stream;
  }
}

#endif // !CORE_TIMESPAN_HPP_INCLUDED
