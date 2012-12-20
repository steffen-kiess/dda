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

#include "GeometryParser.hpp"

#include <Core/ParsingUtil.hpp>
#include <Core/OStream.hpp>
#include <Core/HelpResultException.hpp>

#include <Math/Vector3IOS.hpp>
#include <Math/DiagMatrix3.hpp>

#include <EMSim/Parse.hpp>

#include <DDA/Shapes.hpp>

#include <cstring>
#include <cstdlib>
#include <map>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

namespace DDA {
  typedef std::map<std::string, boost::shared_ptr<const GeometryParser> > MapType;
  typedef std::pair<std::string, boost::shared_ptr<const GeometryParser> > PairType;

  static MapType createParsers () {
    MapType parsers;
#define addvalue(x)                                             \
    do {                                                        \
      const boost::shared_ptr<const GeometryParser> p = (x);    \
      ASSERT (parsers.find (p->name ()) == parsers.end ());     \
      parsers[p->name ()] = p;                                  \
    } while (0)
#define add(x)                                                  \
    extern boost::shared_ptr<const GeometryParser> x ();        \
    addvalue (x ())

    addvalue (Shapes::sphereParser ());
    addvalue (Shapes::cylinderParser ());
    addvalue (Shapes::boxParser ());
#ifdef GEOMETRYPARSER_ADDITIONAL_PARSERS
    GEOMETRYPARSER_ADDITIONAL_PARSERS
#endif

#undef add
#undef addvalue
      return parsers;
  }

  static const MapType& getParsers () {
    static MapType parsers = createParsers ();
    return parsers;
  }

  GeometryParser::~GeometryParser () {}

  boost::shared_ptr<Geometry> parseGeometry (const std::string& s) {
    size_t pos = 0;
    while (pos < s.length () && s[pos] >= 'a' && s[pos] <= 'z')
      pos++;
    std::string name = s.substr (0, pos);
    while (pos < s.length () && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == ':'))
      pos++;
    std::string value = s.substr (pos);
    if (name == "help") {
      std::stringstream str;
      str << "Available geometry options:" << std::endl;
      str << "  load:file=<filename>" << std::endl;
      BOOST_FOREACH (const PairType& pair, getParsers ()) {
        str << "  " << pair.first << ":" << pair.second->info () << std::endl;
      }
      throw Core::HelpResultException (str.str ());
    } else if (name == "load") {
      return boost::shared_ptr<Geometry> ();
    } else {
      const MapType& map = getParsers ();
      MapType::const_iterator it = map.find (name);
      if (it == map.end ()) {
        ABORT_MSG ("Unknown geometry option `" + name + "'");
      } else {
        return it->second->parse (value);
      }
    }
  }
}
