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

#ifndef MATH_FPTEMPLATEINSTANCES_HPP_INCLUDED
#define MATH_FPTEMPLATEINSTANCES_HPP_INCLUDED

// Macros to instanciate a template for all float types

#include <Core/Util.hpp>

#define CREATE_TEMPLATE_INSTANCE(Class, Template) template class Template<Class>;
#define CREATE_TEMPLATE_INSTANCE_S(Class, Template) template struct Template<Class>;

#if HAVE_CXX11
#define DISABLE_TEMPLATE_INSTANCE(Class, Template) extern template class Template<Class>;
#else
#define DISABLE_TEMPLATE_INSTANCE(Class, Template)
#endif

#define CALL_MACRO_FOR_DEFAULT_FP_TYPES(M, A)   \
  M(float, A) M(double, A) M(long double, A) // M(__float128, A)

#define CALL_MACRO_FOR_OPENCL_FP_TYPES(M, A)   \
  M(float, A) M(double, A)

#endif // !MATH_FPTEMPLATEINSTANCES_HPP_INCLUDED
