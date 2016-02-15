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

#ifndef CORE_TYPE_HPP_INCLUDED
#define CORE_TYPE_HPP_INCLUDED

// Methods to get a typename or to get the offset of a field in a type

#include <Core/Util.hpp>

#include <string>
#include <typeinfo>

#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_volatile.hpp>

#include <boost/type_traits/alignment_of.hpp>
#include <boost/type_traits/aligned_storage.hpp>

namespace Core {
  namespace Type {
    std::string getName (const std::type_info& info);

    template <typename T> inline std::string getName () {
      std::string result = getName (typeid (T));
      if (boost::is_const<T>::value)
        result += " const";
      if (boost::is_volatile<T>::value)
        result += " volatile";
      return result;
    }

    // Return the offset of a pointer-to-member
    // Might cause problems for types with a vtable pointer (in particular for
    // types with virtual inheritance)
    template <typename C, typename V>
    NVCC_HOST_DEVICE static inline size_t getOffset (V C::* ptr) {
#if HAVE_CXX11
      static_assert (std::is_standard_layout<C>::value, "C is not a standard layout type");
#endif
      // This works, but is not portable
      // return ((char*) &(((C*) 0)->*ptr)) - ((char*) 0);

      // This should be more or less portable. The compiler should optimize this
      // to the above form
      typename boost::aligned_storage<sizeof (V), boost::alignment_of<V>::value>::type val;
      return ((const char*) &(((const C*) &val)->*ptr)) - ((const char*) &val);
    }
  }
}

#endif // !CORE_TYPE_HPP_INCLUDED
