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

#ifndef DDA_FIELDCALCULATOR_HPP_INCLUDED
#define DDA_FIELDCALCULATOR_HPP_INCLUDED

// Base class for far field calculation on the CPU or on the GPU

#include <Math/Forward.hpp>

#include <Math/FPTemplateInstances.hpp>

#include <DDA/Forward.hpp>

#include <complex>
#include <vector>

namespace DDA {
  template <class T>
  class FieldCalculator {
#ifdef SWIG_VISIBILITY_WORKAROUND
  public:
#endif
    typedef T ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

  private:
    const DDAParams<ftype>& ddaParams_;

  public:
    FieldCalculator (const DDAParams<ftype>& ddaParams);
    virtual ~FieldCalculator ();

    const DDAParams<ftype>& ddaParams () const { return ddaParams_; }

    // pvec must remain valid while the FieldCalculator is used
    virtual void setPVec (const std::vector<ctype>& pvec) = 0;
    virtual Math::Vector3<ctype> calcField (Math::Vector3<ftype> n) = 0;
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES (DISABLE_TEMPLATE_INSTANCE, FieldCalculator)
}

#endif // !DDA_FIELDCALCULATOR_HPP_INCLUDED
