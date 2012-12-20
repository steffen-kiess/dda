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

#ifndef EMSIM_MIE_HPP_INCLUDED
#define EMSIM_MIE_HPP_INCLUDED

// Code for calculating mie solutions for spheres

#include <Math/Float.hpp>

#include <EMSim/Forward.hpp>

#include <complex>

#include <boost/shared_ptr.hpp>

namespace EMSim {
  void BHMie (ldouble x, cldouble refrel, const AngleList& angleList, ldouble farFieldFactor,
              boost::shared_ptr<DataFiles::JonesFarField<ldouble> >& farField, CrossSection& crossSection, ldouble& qback, ldouble& gsca);
}

#endif // !EMSIM_MIE_HPP_INCLUDED
