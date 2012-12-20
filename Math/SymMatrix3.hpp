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

#ifndef MATH_SYMMATRIX3_HPP_INCLUDED
#define MATH_SYMMATRIX3_HPP_INCLUDED

// Math::SymMatrix3<T> is a symmetric 3x3 matrix

#include <Math/Forward.hpp>
#include <Math/Vector3.hpp>

#include <Core/Assert.hpp>

#include <boost/type_traits/alignment_of.hpp>

namespace Math {
  // symmetric 3x3 matrix
  /* (aa0 ab1 ac2)
   * (ab1 bb3 bc4)
   * (ac2 bc4 cc5)
   */
  template <typename T> class /*__attribute__ ((aligned (8 * boost::alignment_of<T>::value)))*/ SymMatrix3 {
    T data_[6];
    //T padding[2];

  public:
    SymMatrix3 () {}
    SymMatrix3 (T aa, T ab, T ac, T bb, T bc, T cc) {
      data_[0] = aa;
      data_[1] = ab;
      data_[2] = ac;
      data_[3] = bb;
      data_[4] = bc;
      data_[5] = cc;
    }

    const T& operator[] (int i) const {
      ASSERT (i >= 0 && i < 6);
      return data_[i];
    }

    T& operator[] (int i) {
      ASSERT (i >= 0 && i < 6);
      return data_[i];
    }

    Math::Vector3<T> getVector (int i) {
      switch (i) {
      case 0: return Math::Vector3<T> (aa (), ab (), ac ());
      case 1: return Math::Vector3<T> (ab (), bb (), bc ());
      case 2: return Math::Vector3<T> (ac (), bc (), cc ());
      default: ABORT ();
      }
    }

#define DCOMP(name, nr)                         \
    const T& name () const {                    \
      return data_[nr];                         \
    }                                           \
    T& name () {                                \
      return data_[nr];                         \
    }
    DCOMP(aa, 0) DCOMP(ab, 1) DCOMP(ac, 2) DCOMP(bb, 3) DCOMP(bc, 4) DCOMP(cc, 5)
#undef DCOMP
  };

  template <typename T> inline Math::Vector3<T> operator* (SymMatrix3<T> m, Math::Vector3<T> v) {
    return Math::Vector3<T> (
                             m.aa()*v.x() + m.ab()*v.y() + m.ac()*v.z(),
                             m.ab()*v.x() + m.bb()*v.y() + m.bc()*v.z(),
                             m.ac()*v.x() + m.bc()*v.y() + m.cc()*v.z()
                             );
  }
}

#endif // !MATH_SYMMATRIX3_HPP_INCLUDED
