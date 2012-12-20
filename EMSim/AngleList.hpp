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

#ifndef EMSIM_ANGLELIST_HPP_INCLUDED
#define EMSIM_ANGLELIST_HPP_INCLUDED

// EMSim::AngleList is a list of (theta,phi) pairs of angles used for far field
// calculations.

#include <Math/Forward.hpp>
#include <Math/Float.hpp>

#include <EMSim/Forward.hpp>
#include <EMSim/Parse.hpp>

#include <utility>
#include <vector>

#include <cstddef>

namespace EMSim {
  class AngleList {
  public:
    virtual ~AngleList ();

    virtual size_t count () const = 0;
    virtual std::pair<ldouble, ldouble> getThetaPhi (size_t i) const = 0;
  };

  class NullAngleList : public AngleList {
  public:
    virtual ~NullAngleList ();

    virtual size_t count () const { return 0; }
    virtual std::pair<ldouble, ldouble> getThetaPhi (size_t i) const;
  };

  class GridAngleList : public AngleList {
    ldouble _theta0;
    ldouble _phi0;
    ldouble _radPerTheta;
    ldouble _radPerPhi;
    size_t _nTheta;
    size_t _nPhi;
    size_t _count;

  public:
    GridAngleList ();
    GridAngleList (ldouble theta0, ldouble phi0, ldouble radPerTheta, ldouble radPerPhi, size_t nTheta, size_t nPhi);
    virtual ~GridAngleList ();

    virtual std::pair<ldouble, ldouble> getThetaPhi (size_t i) const;


    ldouble theta0 () const { return _theta0; }
    ldouble phi0 () const { return _phi0; }
    ldouble radPerTheta () const { return _radPerTheta; }
    ldouble radPerPhi () const { return _radPerPhi; }
    size_t nTheta () const { return _nTheta; }
    size_t nPhi () const { return _nPhi; }
    virtual size_t count () const { return _count; }

    bool operator< (const GridAngleList& other) const;
  };

  void parse (StringParser& p, GridAngleList& out);
}

#endif // !EMSIM_ANGLELIST_HPP_INCLUDED
