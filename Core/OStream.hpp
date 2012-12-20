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

#ifndef CORE_OSTREAM_HPP_INCLUDED
#define CORE_OSTREAM_HPP_INCLUDED

#include "OStream.forward.hpp"

// Core::IStream is a reference-counted output stream
// Unlike std::ostream most operations check the stream state and throw an
// exception in case of an error.
//
// Provides a fprintf method which can be used like
// stream.fprintf ("%s %d\n", "foo", 4);
//
// Core::sprintf () works like stream.fprintf but returns a std::string
//
// EPRINTVALS (foo, bar) will print something like "foo = 4, bar = 5" to stderr.

#include <Core/Assert.hpp>
#include <Core/Util.hpp>

#include <ostream>
#include <sstream>

#include <boost/smart_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>

#if !HAVE_CXX11
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/control/if.hpp>
#endif

namespace Core {
  class OStream {
    boost::shared_ptr<std::ostream> stream;

    struct NoopDeallocator {
      void operator() (UNUSED std::ostream* str) {
      }
    };

    OStream (boost::shared_ptr<std::ostream> stream) : stream (stream) {}

  public:
    OStream (std::ostream* str) : stream (str) {
      ASSERT (str);
    }

    bool good () const {
      //ASSERT (stream);
      return stream->good ();
    }
    void assertGood () const {
      ASSERT (good ());
    }

    std::ostream& operator* () const {
      //ASSERT (stream);
      return *stream;
    }
    std::ostream* operator-> () const {
      //ASSERT (stream);
      return stream.get ();
    }

    template <typename T>
    const OStream& operator << (T& t) const {
      assertGood ();
      **this << t;
      assertGood ();
      return *this;
    }
    template <typename T>
    const OStream& operator << (const T& t) const {
      assertGood ();
      **this << t;
      assertGood ();
      return *this;
    }
    // for i/o manipulators
    const OStream& operator << (std::ostream& (*t)(std::ostream&)) const {
      assertGood ();
      **this << t;
      assertGood ();
      return *this;
    }

  private:
    static const boost::format& getValFormat0 () {
      static boost::format format ("%s = %s, ", std::locale::classic ());
      return format;
    }
    static const boost::format& getValFormat () {
      static boost::format format ("%s = %s\n", std::locale::classic ());
      return format;
    }
    static std::vector<std::string> splitNames (const std::string& str);
  public:

    void fprintfNoCopy (boost::format& format) const {
      *this << format;
    }

    template <typename T>
    void fprintval (const char* name, T value) const {
      fprintf (getValFormat (), name, value);
    }
    template <typename T>
    void fprintvals (const std::vector<std::string>::const_iterator& firstName, const std::vector<std::string>::const_iterator& lastName, T value) const {
      ASSERT (firstName != lastName);
      ASSERT (firstName + 1 == lastName);
      fprintf (getValFormat (), *firstName, value);
    }
#if HAVE_CXX11
    template <typename H, typename... T>
    void fprintfNoCopy (boost::format& format, const H& head, const T&... tail) const {
      format % head;
      fprintfNoCopy (format, tail...);
    }

    template <typename... T>
    void fprintf (const boost::format& formatRef, T... param) const {
      boost::format format (formatRef);
      fprintfNoCopy (format, param...);
    }
    template <typename... T>
    void fprintf (const char* str, T... param) const {
      boost::format format (str, std::locale::classic ());
      fprintfNoCopy (format, param...);
    }
    template <typename... T>
    void fprintfL (const char* str, T... param) const {
      boost::format format (str);
      fprintfNoCopy (format, param...);
    }
    template <typename... T>
    void fprintf (const char* str, const std::locale& loc, T... param) const {
      boost::format format (str, loc);
      fprintfNoCopy (format, param...);
    }
    template <typename... T>
    void fprintf (const std::string& str, T... param) const {
      boost::format format (str, std::locale::classic ());
      fprintfNoCopy (format, param...);
    }
    template <typename... T>
    void fprintfL (const std::string& str, T... param) const {
      boost::format format (str);
      fprintfNoCopy (format, param...);
    }
    template <typename... T>
    void fprintf (const std::string& str, const std::locale& loc, T... param) const {
      boost::format format (str, loc);
      fprintfNoCopy (format, param...);
    }

    template <typename T, typename U0, typename... U>
    void fprintvals (const std::vector<std::string>::const_iterator& firstName, const std::vector<std::string>::const_iterator& lastName, T value, U0 values0, U... values) const {
      ASSERT (firstName != lastName);
      fprintf (getValFormat0 (), *firstName, value);
      fprintvals (firstName + 1, lastName, values0, values...);
    }
    template <typename T0, typename... T>
    void fprintvals (const std::string& names, T0 values0, T... values) const {
      std::vector<std::string> namesV = splitNames (names);
      fprintvals (namesV.begin (), namesV.end (), values0, values...);
    }
#else
#define TEMPLATE(n) BOOST_PP_IF (n, template <, public:/*To avoid warnings*/) \
               BOOST_PP_ENUM_PARAMS (n, typename T)                     \
               BOOST_PP_IF (n, >, public:/*To avoid warnings*/)
#define DEFINE_OVERLOADS(n)                                             \
    template <typename H  BOOST_PP_ENUM_TRAILING_PARAMS (n, typename T)> \
    void fprintfNoCopy (boost::format& format, const H& head  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const T, & v)) const { \
      format % head;                                                    \
      fprintfNoCopy (format  BOOST_PP_ENUM_TRAILING_PARAMS (n, v));     \
    }                                                                   \
                                                                        \
    TEMPLATE(n)                                                         \
    void fprintf (const boost::format& formatRef  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const T, & v)) const { \
      boost::format format (formatRef);                                 \
      fprintfNoCopy (format  BOOST_PP_ENUM_TRAILING_PARAMS (n, v));     \
    }                                                                   \
    TEMPLATE(n)                                                         \
    void fprintf (const char* str  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const T, & v)) const { \
      boost::format format (str, std::locale::classic ());              \
      fprintfNoCopy (format  BOOST_PP_ENUM_TRAILING_PARAMS (n, v));     \
    }                                                                   \
    TEMPLATE(n)                                                         \
    void fprintfL (const char* str  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const T, & v)) const { \
      boost::format format (str);                                       \
      fprintfNoCopy (format  BOOST_PP_ENUM_TRAILING_PARAMS (n, v));     \
    }                                                                   \
    TEMPLATE(n)                                                         \
    void fprintf (const char* str, const std::locale& loc  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const T, & v)) const { \
      boost::format format (str, loc);                                  \
      fprintfNoCopy (format  BOOST_PP_ENUM_TRAILING_PARAMS (n, v));     \
    }                                                                   \
    TEMPLATE(n)                                                         \
    void fprintf (const std::string& str  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const T, & v)) const { \
      boost::format format (str, std::locale::classic ());              \
      fprintfNoCopy (format  BOOST_PP_ENUM_TRAILING_PARAMS (n, v));     \
    }                                                                   \
    TEMPLATE(n)                                                         \
    void fprintfL (const std::string& str  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const T, & v)) const { \
      boost::format format (str);                                       \
      fprintfNoCopy (format  BOOST_PP_ENUM_TRAILING_PARAMS (n, v));     \
    }                                                                   \
    TEMPLATE(n)                                                         \
    void fprintf (const std::string& str, const std::locale& loc  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const T, & v)) const { \
      boost::format format (str, loc);                                  \
      fprintfNoCopy (format  BOOST_PP_ENUM_TRAILING_PARAMS (n, v));     \
    }                                                                   \
                                                                        \
    template <typename T, typename UU0  BOOST_PP_ENUM_TRAILING_PARAMS (n, typename U)> \
    void fprintvals (const std::vector<std::string>::const_iterator& firstName, const std::vector<std::string>::const_iterator& lastName, T value, const UU0& values0  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const U, & v)) const { \
      ASSERT (firstName != lastName);                                   \
      fprintf (getValFormat0 (), *firstName, value);                    \
      fprintvals (firstName + 1, lastName, values0   BOOST_PP_ENUM_TRAILING_PARAMS (n, v)); \
    }                                                                   \
    template <typename TT0  BOOST_PP_ENUM_TRAILING_PARAMS (n, typename T)> \
    void fprintvals (const std::string& names, const TT0& values0  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const T, & v)) const { \
      std::vector<std::string> namesV = splitNames (names);             \
      fprintvals (namesV.begin (), namesV.end (), values0  BOOST_PP_ENUM_TRAILING_PARAMS (n, v)); \
    }                                                                   \
    
#define DEFINE_OVERLOADS2(z, i, data) DEFINE_OVERLOADS (i)
    // Allow up to 9 parameters
    BOOST_PP_REPEAT (10, DEFINE_OVERLOADS2, NOTHING)
#undef DEFINE_OVERLOADS2
#undef DEFINE_OVERLOADS
#undef TEMPLATE
#endif

    static OStream get (std::ostream& str) {
      return OStream (boost::shared_ptr<std::ostream> (&str, NoopDeallocator ()));
    }
    static OStream getStdout ();
    static OStream getStderr ();

    static OStream tee (const OStream& s1, const OStream& s2);
    static OStream open (const boost::filesystem::path& path, std::ios_base::openmode mode = std::ios_base::out);
    static OStream openNull ();
  };

#if HAVE_CXX11
  template <typename... T>
  inline std::string sprintf (const std::string& str, T... param) {
    std::stringstream s;
    OStream::get (s).fprintf (str, param...);
    return s.str ();
  }
  template <typename... T>
  inline std::string sprintf (const char* str, T... param) {
    std::stringstream s;
    OStream::get (s).fprintf (str, param...);
    return s.str ();
  }
#else
#define TEMPLATE(n) BOOST_PP_IF (n, template <, namespace{}/*To avoid warnings*/) \
  BOOST_PP_ENUM_PARAMS (n, typename T)                                  \
  BOOST_PP_IF (n, >, namespace{}/*To avoid warnings*/)
#define DEFINE_OVERLOADS(n)                                             \
  TEMPLATE(n)                                                           \
  inline std::string sprintf (const std::string& str  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const T, & v)) { \
    std::stringstream s;                                                \
    OStream::get (s).fprintf (str  BOOST_PP_ENUM_TRAILING_PARAMS (n, v)); \
    return s.str ();                                                    \
  }                                                                     \
  TEMPLATE(n)                                                           \
  inline std::string sprintf (const char* str  BOOST_PP_ENUM_TRAILING_BINARY_PARAMS (n, const T, & v)) { \
    std::stringstream s;                                                \
    OStream::get (s).fprintf (str  BOOST_PP_ENUM_TRAILING_PARAMS (n, v)); \
    return s.str ();                                                    \
  }                                                                     \
    
#define DEFINE_OVERLOADS2(z, i, data) DEFINE_OVERLOADS (i)
  // Allow up to 9 parameters
  BOOST_PP_REPEAT (10, DEFINE_OVERLOADS2, NOTHING)
#undef DEFINE_OVERLOADS2
#undef DEFINE_OVERLOADS
#undef TEMPLATE
#endif
}

#define FPRINTF(stream, formatString, ...) do {                         \
    static boost::format FPRINTF_format ("" formatString "", std::locale::classic ()); \
    (stream).fprintf (FPRINTF_format, __VA_ARGS__);                     \
  } while (0)
#define FPRINTF0(stream, formatString) do {                             \
    static boost::format FPRINTF_format ("" formatString "", std::locale::classic ()); \
    (stream).fprintf (FPRINTF_format);                                  \
  } while (0)

#define FPRINTFL(stream, formatString, ...) do {                \
    static boost::format FPRINTF_format ("" formatString "");   \
    (stream).fprintf (FPRINTF_format, __VA_ARGS__);             \
  } while (0)
#define FPRINTFL0(stream, formatString) do {                    \
  static boost::format FPRINTF_format ("" formatString "");     \
  (stream).fprintf (FPRINTF_format);                            \
  } while (0)

#define FPRINTVAL(stream, value) (stream).fprintval (#value, value)
#define EPRINTVAL(value) Core::OStream::getStderr ().fprintval (#value, value)

#define FPRINTVALS(stream, ...) (stream).fprintvals (#__VA_ARGS__, __VA_ARGS__)
#define EPRINTVALS(...) Core::OStream::getStderr ().fprintvals (#__VA_ARGS__, __VA_ARGS__)

#endif // !CORE_OSTREAM_HPP_INCLUDED
