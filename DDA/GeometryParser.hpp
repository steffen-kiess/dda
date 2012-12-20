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

#ifndef DDA_GEOMETRYPARSER_HPP_INCLUDED
#define DDA_GEOMETRYPARSER_HPP_INCLUDED

// Code for parsing a geometry string

#include <Core/Assert.hpp>

#include <DDA/Forward.hpp>

#include <boost/shared_ptr.hpp>

#include <string>

namespace DDA {
  class GeometryParser {
  private:
    std::string name_;
    std::string info_;

  public:
    GeometryParser () {}
    GeometryParser (const std::string& name, const std::string& info) : name_ (name), info_ (info) {}
    ~GeometryParser ();

    const std::string& name () const { return name_; }
    const std::string& info () const { return info_; }
  
    virtual boost::shared_ptr<Geometry> parse (const std::string& s) const = 0;
  };

#define GEOMETRYPARSER_CREATE_IMPL(method, type, name, info)            \
  boost::shared_ptr<const GeometryParser> method () {                   \
    class GeometryParserImpl : public GeometryParser {                  \
    public:                                                             \
    GeometryParserImpl () : GeometryParser (name, info) {}              \
    virtual boost::shared_ptr<Geometry> parse (const std::string& s) const { \
      return type::parse (s);                                           \
    }                                                                   \
    };                                                                  \
    GeometryParser* pointer = new GeometryParserImpl ();                \
    static boost::shared_ptr<const GeometryParser> parser (pointer);    \
    return parser;                                                      \
  }

  boost::shared_ptr<Geometry> parseGeometry (const std::string& s);
}

#endif // !DDA_GEOMETRYPARSER_HPP_INCLUDED
