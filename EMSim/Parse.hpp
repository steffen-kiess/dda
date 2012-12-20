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

#ifndef EMSIM_PARSE_HPP_INCLUDED
#define EMSIM_PARSE_HPP_INCLUDED

// Code for parsing values from the command line

#include <Core/Assert.hpp>
#include <Core/Exception.hpp>
#include <Core/Type.hpp>

#include <typeinfo>
#include <sstream>
#include <istream>
#include <complex>

#include <Math/Vector3.hpp>
#include <Math/DiagMatrix3.hpp>

namespace EMSim {
  class ParseException : public Core::Exception {
    const std::type_info& type_;
    std::string raw_;
    size_t start_;
    size_t errorPos_;
    std::string error_;

  public:
    ParseException (const std::type_info& type, std::string raw, size_t start, size_t errorPos, std::string error) : type_ (type), raw_ (raw), start_ (start), errorPos_ (errorPos), error_ (error) {}
    virtual ~ParseException () throw () {}

    const std::type_info& type () const { return type_; }
    const std::string& raw () const { return raw_; }
    size_t start () const { return start_; }
    size_t errorPos () const { return errorPos_; }
    const std::string& error () const { return error_; }

    virtual std::string message () const {
      std::stringstream str;
      str << "Could not parse string `" << raw ().substr (start ()) << "' as " << Core::Type::getName (type ()) << ": at col " << (errorPos () - start ()) << ": " << error ();
      return str.str ();
    }
  };

  class StringParser {
    std::string value_;
    size_t pos_;

  public:
    StringParser (const std::string& value) : value_ (value), pos_ (0) {}

    const std::string& value () const { return value_; }
    size_t pos () const { return pos_; }

    bool eof () const { return pos () >= value ().length (); }
    char peek () const { return eof () ? '\0' : value ()[pos ()]; }

    std::string remaining () const {
      return value ().substr (pos ());
    }

    char read () {
      if (pos () >= value ().length ()) {
        return '\0';
      } else {
        char c = value ()[pos ()];
        pos_++;
        return c;
      }
    }
    void rewind (size_t count = 1) {
      ASSERT (pos () >= count);
      pos_ -= count;
    }
    void skip (size_t count) {
      ASSERT (pos () + count <= value ().length ());
      pos_ += count;
    }
    void seek (size_t newpos) {
      ASSERT (newpos <= value ().length ());
      pos_ = newpos;
    }

    void skipWs () {
      while (peek () == ' ' || peek () == '\t' || peek () == '\r' || peek () == '\n')
        read ();
    }
    void expect (const std::type_info& type, size_t start, char expected) {
      skipWs ();
      if (eof ()) {
        std::stringstream str;
        str << "Expected `" << expected << "', got EOF";
        throw ParseException (type, value (), start, pos (), str.str ());
      }
      char c = read ();
      if (c != expected) {
        std::stringstream str;
        str << "Expected `" << expected << "', got `" << c << "'";
        throw ParseException (type, value (), start, pos () - 1, str.str ());
      }
    }
  };

  template <typename T>
  void parse (const std::string& s, T& out) {
    StringParser p (s);
    parse (p, out);
    p.skipWs ();
    if (!p.eof ())
      throw ParseException (typeid (T), p.value (), 0, p.pos (), "Got garbage at end of string");
  }

  template <typename T>
  void parse (StringParser& p, T& out) {
    size_t start = p.pos ();
    std::istringstream stream (p.value ().substr (start));
    std::istream& stream2 (stream);
    stream2 >> out;
    if (stream.fail ()) {
      stream.clear ();
      std::streampos pos = stream.tellg ();
      ASSERT (pos >= 0);
      throw ParseException (typeid (T), p.value (), start, start + (size_t) pos, "operator>> failed");
    }
    stream.clear (); // Needed (for clearing EOF bit?), otherwise stream.tellg() fails (returns -1)
    std::streampos pos = stream.tellg ();
    ASSERT (pos >= 0);
    p.skip ((size_t) pos);
  }

  template <typename F>
  void parse (StringParser& p, std::complex<F>& out) {
    size_t start = p.pos ();
    F real, imag;
    parse (p, real);
    size_t pos = p.pos ();
    p.skipWs ();
    size_t pos2 = p.pos ();
    char sign = p.read ();
    if (sign == '+' || sign == '-') {
      p.skipWs ();
      if (p.peek () == 'i' || p.peek () == 'I') {
        p.read ();
        imag = sign == '+' ? 1 : -1;
      } else {
        p.seek (pos2);
        parse (p, imag);
        p.skipWs ();
        char c = p.read ();
        if (c != 'i' && c != 'I') {
          throw ParseException (typeid (std::complex<F>), p.value (), start, p.pos () - 1, "Expected `i' oder `I' after imaginary part");
        }
      }
      out = std::complex<F> (real, imag);
    } else {
      p.seek (pos);
      out = std::complex<F> (real, 0);
    }
  }

  template <typename T>
  void parse (StringParser& p, Math::Vector3<T>& out) {
    size_t start = p.pos ();
    p.expect (typeid (Math::Vector3<T>), start, '(');
    parse (p, out.x ());
    p.expect (typeid (Math::Vector3<T>), start, ',');
    parse (p, out.y ());
    p.expect (typeid (Math::Vector3<T>), start, ',');
    parse (p, out.z ());
    p.expect (typeid (Math::Vector3<T>), start, ')');
  }

  template <typename T>
  void parse (StringParser& p, Math::DiagMatrix3<T>& out) {
    size_t start = p.pos ();
    if (p.peek () == '(') {
      p.expect (typeid (Math::DiagMatrix3<T>), start, '(');
      parse (p, out.m11 ());
      p.expect (typeid (Math::DiagMatrix3<T>), start, ',');
      parse (p, out.m22 ());
      p.expect (typeid (Math::DiagMatrix3<T>), start, ',');
      parse (p, out.m33 ());
      p.expect (typeid (Math::DiagMatrix3<T>), start, ')');
    } else {
      T value;
      parse (p, value);
      out = Math::DiagMatrix3<T> (value, value, value);
    }
  }
}

#endif // !EMSIM_PARSE_HPP_INCLUDED
