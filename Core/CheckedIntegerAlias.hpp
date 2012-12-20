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

#ifndef CORE_CHECKEDINTEGERALIAS_HPP_INCLUDED
#define CORE_CHECKEDINTEGERALIAS_HPP_INCLUDED

// This header defines some aliases for checked integer types in the
// root namespace

#include <Core/CheckedInteger.hpp>

#define DS2(t, n) typedef Core::CheckedInteger<t> c##n;
#define DS(t) DS2 (t, t)
DS (int8_t)  DS (uint8_t)
DS (int16_t) DS (uint16_t)
DS (int32_t) DS (uint32_t)
DS (int64_t) DS (uint64_t)
DS (ptrdiff_t) DS (size_t)
DS (intptr_t) DS (uintptr_t)
DS (int) DS2 (unsigned int, uint)
DS (long) DS2 (unsigned long, ulong)
DS2 (long long, longlong) DS2 (unsigned long long, ulonglong)
#undef DS
#undef DS2

#endif // !CORE_CHECKEDINTEGERALIAS_HPP_INCLUDED
