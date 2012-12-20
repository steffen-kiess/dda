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

#include "ToString.hpp"

#include <EMSim/Length.hpp>

#include <boost/foreach.hpp>

namespace DDA {
  std::vector<std::string> anyToStringList (const boost::any& p) {
    std::vector<std::string> s;
    if (0) {}
#define P(x)                                                            \
    else if (boost::any_cast<x> (&p)) s.push_back (toString (boost::any_cast<x> (p))); \
    else if (boost::any_cast<std::vector<x> > (&p)) { const std::vector<x>& ve = boost::any_cast<std::vector<x> > (p); BOOST_FOREACH (const x& r, ve) s.push_back (toString (r)); }
#define P2(x) P(x) P(Math::Vector3 < x >) P(Math::DiagMatrix3 < x >)
    P(std::string)
      P2(ldouble)
      P2(cldouble)
      P2(EMSim::Length)
      P2(uint32_t)
#undef P
#undef P2
    else
      s.push_back ("..." + Core::Type::getName (p.type ()));
    return s;
  }
}
