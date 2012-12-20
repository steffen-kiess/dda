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

#include "AngleList.hpp"

#include <Core/Assert.hpp>
#include <Core/CheckedIntegerAlias.hpp>

#include <Math/Vector3.hpp>
#include <Math/Vector3IOS.hpp>

#include <algorithm>

#include <boost/lexical_cast.hpp>
#include <boost/math/constants/constants.hpp>

namespace EMSim {
  AngleList::~AngleList () {}

  NullAngleList::~NullAngleList () {}
  NORETURN_ATTRIBUTE std::pair<ldouble, ldouble> NullAngleList::getThetaPhi (UNUSED size_t i) const {
    ABORT ();
  }


  GridAngleList::GridAngleList () :
    _theta0 (0),
    _phi0 (0),
    _radPerTheta (0),
    _radPerPhi (0),
    _nTheta (0),
    _nPhi (0)
  {
    _count = 0;
  }

  GridAngleList::GridAngleList (ldouble theta0, ldouble phi0, ldouble radPerTheta, ldouble radPerPhi, size_t nTheta, size_t nPhi) :
    _theta0 (theta0),
    _phi0 (phi0),
    _radPerTheta (radPerTheta),
    _radPerPhi (radPerPhi),
    _nTheta (nTheta),
    _nPhi (nPhi)
  {
    //_radPerTheta = boost::math::constants::pi<ldouble> () / (_nTheta - 1);
    //_radPerPhi = 2 * boost::math::constants::pi<ldouble> () / (_nPhi - 1);
    _count = (csize_t (_nTheta) * csize_t (_nPhi)) ();
  }

  GridAngleList::~GridAngleList () {}

  std::pair<ldouble, ldouble> GridAngleList::getThetaPhi (size_t i) const {
    ASSERT (i < _count);
    return std::make_pair ((i / nPhi ()) * radPerTheta () + theta0 (), (i % nPhi ()) * radPerPhi () + phi0 ());
  }

  namespace {
    void parseAngles (const std::string& str, ldouble& angl0, ldouble& step, size_t& count) {
      size_t dotDot = str.find ("..");
      if (dotDot == std::string::npos) {
        parse (str, angl0);
        angl0 *= boost::math::constants::pi<ldouble> () / 180;
        step = 0;
        count = 1;
        return;
      }
    
      ldouble start, end;
      size_t n;
      parse (str.substr (0, dotDot), start);
      std::string s = str.substr (dotDot + 2);
      size_t slash = s.find ("/");
      ASSERT (slash != std::string::npos);
      parse (s.substr (0, slash), end);
      parse (s.substr (slash + 1), n);
      start *= boost::math::constants::pi<ldouble> () / 180;
      end *= boost::math::constants::pi<ldouble> () / 180;
      angl0 = start;
      count = n;
      if (count == 0) {
        step = 0;
      } else if (count == 1) {
        ASSERT (start == end);
        step = 0;
      } else {
        step = (end - start) / (n - 1);
      }
    }
  }

  bool GridAngleList::operator< (const GridAngleList& other) const {
#define COMP(x)                                 \
    if (x () < other.x ())                      \
      return true;                              \
    else if (x () > other.x ())                 \
      return false;
    COMP (theta0) COMP (phi0);
    COMP (radPerTheta) COMP (radPerPhi);
    COMP (nTheta) COMP (nPhi);
    return false;
#undef COMP
  }

  void parse (StringParser& p, GridAngleList& out) {
    std::string s1;
    while (!p.eof () && p.peek () != ',')
      s1 += p.read ();
    ASSERT (p.read () == ',');
    std::string s2;
    while (!p.eof () && p.peek () != ',')
      s2 += p.read ();
  
    ldouble theta0;
    ldouble phi0;
    ldouble radPerTheta;
    ldouble radPerPhi;
    size_t nTheta;
    size_t nPhi;
    parseAngles (s1, theta0, radPerTheta, nTheta);
    parseAngles (s2, phi0, radPerPhi, nPhi);
    out = GridAngleList (theta0, phi0, radPerTheta, radPerPhi, nTheta, nPhi);
  }
}
