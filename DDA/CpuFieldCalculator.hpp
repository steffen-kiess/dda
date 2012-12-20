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

#ifndef DDA_CPUFIELDCALCULATOR_HPP_INCLUDED
#define DDA_CPUFIELDCALCULATOR_HPP_INCLUDED

// Implementation of far field calculation on the CPU

#include <DDA/FieldCalculator.hpp>

#include <vector>

namespace DDA {
  template <class T>
  class CpuFieldCalculator : public FieldCalculator<T> {
    typedef T ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

    using FieldCalculator<T>::ddaParams;

    std::vector<ctype> pvec;
    std::vector<ctype> xValues;

  public:
    CpuFieldCalculator (const DDAParams<ftype>& ddaParams);
    virtual ~CpuFieldCalculator ();

    virtual void setPVec (const std::vector<ctype>& pvec);
    virtual Math::Vector3<ctype> calcField (Math::Vector3<ftype> n);
  };

  //CALL_MACRO_FOR_DEFAULT_FP_TYPES (DISABLE_TEMPLATE_INSTANCE, CpuFieldCalculator)
}

#endif // !DDA_CPUFIELDCALCULATOR_HPP_INCLUDED
