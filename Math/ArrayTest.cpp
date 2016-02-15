/*
 * Copyright (c) 2013 Steffen Kieß
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

#include <Core/OStream.hpp>
#include <Core/Type.hpp>
#include <Core/Exception.hpp>

#include <Math/Array.hpp>

//template class Math::Array<int, 0>;
template class Math::Array<int, 1>;
#if HAVE_CXX11 || defined(ARRAY_OMIT_ONEDIM_CTOR_OVERLOADS) // The constructors / recreate overloads with one std::size_t argument cannot be instanciated for pre-c++11
template class Math::Array<int, 2>;
template class Math::Array<int, 3>;
#endif

template class Math::Array<bool, 1>;
#if HAVE_CXX11 || defined(ARRAY_OMIT_ONEDIM_CTOR_OVERLOADS) // The constructors / recreate overloads with one std::size_t argument cannot be instanciated for pre-c++11
template class Math::Array<bool, 2>;
template class Math::Array<bool, 3>;
#endif

template class Math::ArrayView<int, 1, false>;
template class Math::ArrayView<int, 2, false>;
template class Math::ArrayView<int, 3, false>;
template class Math::ArrayView<int, 1, true>;
template class Math::ArrayView<int, 2, true>;
template class Math::ArrayView<int, 3, true>;

template class Math::ArrayView<bool, 1, false>;
template class Math::ArrayView<bool, 2, false>;
template class Math::ArrayView<bool, 3, false>;
template class Math::ArrayView<bool, 1, true>;
template class Math::ArrayView<bool, 2, true>;
template class Math::ArrayView<bool, 3, true>;

template class Math::ArrayView<const int, 1, false>;
template class Math::ArrayView<const int, 2, false>;
template class Math::ArrayView<const int, 3, false>;
template class Math::ArrayView<const int, 1, true>;
template class Math::ArrayView<const int, 2, true>;
template class Math::ArrayView<const int, 3, true>;

template class Math::ArrayView<const bool, 1, false>;
template class Math::ArrayView<const bool, 2, false>;
template class Math::ArrayView<const bool, 3, false>;
template class Math::ArrayView<const bool, 1, true>;
template class Math::ArrayView<const bool, 2, true>;
template class Math::ArrayView<const bool, 3, true>;

/*
//template class Math::Array<volatile int, 0>;
template class Math::Array<volatile int, 1>;
template class Math::Array<volatile int, 2>;
template class Math::Array<volatile int, 3>;

template class Math::Array<volatile bool, 1>;
template class Math::Array<volatile bool, 2>;
template class Math::Array<volatile bool, 3>;
*/

template <std::size_t dim>
static void foo (const Math::ArrayView<const int, dim>& view) {
//static void foo (const Math::ArrayView<int, 3>& view) {
  EPRINTVALS (view.dimension ());
  for (size_t i = 0; i < view.dimension (); i++) {
    EPRINTVALS (i, view.shape ()[i], view.stridesBytes ()[i]);
  }
}

struct R {
  operator Math::Range () const {
    return Math::Range (1, 2);
  }
};

int main2 ();

#ifdef ARRAY_OMIT_ONEDIM_CTOR_OVERLOADS
int main2 ()
#else
int main ()
#endif
{
  //const int* q = NULL;
  int* q = NULL;
  size_t a[] = { 4, 5, 6 };
  ptrdiff_t b[] = { 4, 5, 6 };
  Math::ArrayView<const int> cav (q, a, b);
  Math::ArrayView<int> av (q, a, b);
  const Math::ArrayView<int>& avr = av;
  avr.data ();
  Math::Array<int, 3> ar (a);
  ar.setTo (55);
  const Math::Array<int, 3>& arc = ar;

  /*
  EPRINTVALS (ar.dimension ());
  for (size_t i = 0; i < ar.dimension (); i++) {
    EPRINTVALS (i, ar.shape ()[i], ar.stridesBytes ()[i]);
    EPRINTVALS (i, arc.shape ()[i], arc.stridesBytes ()[i]);
  }
  */
  ASSERT (ar.dimension () == 3);
  ASSERT (ar.shape ()[2] == 6);
  ASSERT (ar.stridesBytes ()[2] == 80);
  //foo (ar);
  ar.view ()[1][2][3] = 4;
  ar[3][2][3] = 5;
  //foo (ar.view ());
  //foo (ar.view ()[1]);
  //foo (ar.view ()[1][2]);
  //EPRINTVALS (ar[1][2][3]);
  ASSERT (ar[1][2][3] == 4);
  //EPRINTVALS (ar.view ()[3][2][3]);
  ASSERT (ar.view ()[3][2][3] == 5);

  //const Math::Range r_ (3, 5);
  const Math::Range r_ (0, 4);
  const Math::Range& r = r_;
  //EPRINTVALS (ar.view ()(1, 2, 3));
  ASSERT (ar.view ()(1, 2, 3) == 4);
  //EPRINTVALS (Core::Type::getName (typeid (ar.view ()(r, r, r))));
  //EPRINTVALS (ar.view ()(r, r, r)[0][0][0]);
  ASSERT (ar.view ()(r, r, r)[0][0][0] == 55);
  //EPRINTVALS (ar.view ()(Math::range (1, 1), Math::range (2, 1), Math::range (2, 2))[0][0][1]);
  ASSERT (ar.view ()(Math::range (1, 1), Math::range (2, 1), Math::range (2, 2))[0][0][1] == 4);
  //EPRINTVALS (ar.view ()(Math::range (0, 4), 2, Math::range (0, 6))[1][3]);
  ASSERT (ar.view ()(Math::range (0, 4), 2, Math::range (0, 6))[1][3] == 4);
  //EPRINTVALS (ar.view ()(Math::range (0, 4), Math::range (1), Math::range (0, 6))[1][1][3]);
  ASSERT (ar.view ()(Math::range (0, 4), Math::range (1), Math::range (0, 6))[1][1][3] == 4);

  //EPRINTVALS (ar.view ()(1, "asd", 3));
  //EPRINTVALS (ar.view ()(1, R (), 3)[1]);
  ASSERT (ar.view ()(1, R (), 3)[1] == 4);
  ar.view ()(1, R (), 3)[1]++;
  //EPRINTVALS (ar.view ()(1, R (), 3)[1]);
  ASSERT (ar.view ()(1, R (), 3)[1] == 5);
  ar.view ()(1, 2, 3)++;
  //EPRINTVALS (ar.view ()(1, R (), 3)[1]);
  ASSERT (ar.view ()(1, R (), 3)[1] == 6);
  //EPRINTVALS (ar.view ()(1, R ())[1]);

  //EPRINTVALS (ar(1, R (), 3)[1]);
  ASSERT (ar(1, R (), 3)[1] == 6);
  ar(1, R (), 3)[1]++;
  //EPRINTVALS (ar(1, R (), 3)[1]);
  ASSERT (ar(1, R (), 3)[1] == 7);
  //EPRINTVALS (arc(1, R (), 3)[1]);
  ASSERT (arc(1, R (), 3)[1] == 7);

  /*
  Math::Array<char, 2> a2 (3, 4);
  for (size_t i = 0; i < a2.dimension (); i++)
    EPRINTVALS (i, a2.shape ()[i], a2.stridesBytes ()[i]);

  Math::Array<char, 3> a3 (3, 4, 5);
  for (size_t i = 0; i < a3.dimension (); i++)
    EPRINTVALS (i, a3.shape ()[i], a3.stridesBytes ()[i]);
  */

  Math::Array<int, 3> copy (ar);
  Math::Array<int, 3> copy2 (ar.view ());

  ASSERT (ar.dimension () == copy.dimension ());
  ASSERT (ar.dimension () == copy2.dimension ());
  for (size_t dim = 0; dim < ar.dimension (); dim++) {
    ASSERT (ar.shape ()[dim] == copy.shape ()[dim]);
    ASSERT (ar.shape ()[dim] == copy2.shape ()[dim]);
  }

  ASSERT (copy2.dimension () == 3);
  for (size_t x = 0; x < copy2.shape ()[0]; x++) {
    for (size_t y = 0; y < copy2.shape ()[1]; y++) {
      for (size_t z = 0; z < copy2.shape ()[2]; z++) {
        //EPRINTVALS (x, y, z, ar (x, y, z), copy2 (x, y, z));
        ASSERT (ar (x, y, z) == copy (x, y, z));
        ASSERT (ar (x, y, z) == copy2 (x, y, z));
      }
    }
  }

  copy2.setTo (123);

  ASSERT (copy2.dimension () == 3);
  for (size_t x = 0; x < copy2.shape ()[0]; x++) {
    for (size_t y = 0; y < copy2.shape ()[1]; y++) {
      for (size_t z = 0; z < copy2.shape ()[2]; z++) {
        ASSERT (copy2 (x, y, z) == 123);
      }
    }
  }

  //ar.view () = ar.view ();
  //cav = av;
  //av = av;

  Math::Array<int, 3> arX;
  
  EPRINTVALS (arX.data ());

  arX.recreate (4, 5, 6);

  Math::ArrayView<int, 3> arXC (arX);

  EPRINTVALS (arX.data ());

  bool success = true;

  try {
    arX(1, 1, 6);
    success = false;
  } catch (Core::Exception& e) {
  }
  ASSERT (success);

  try {
    arX[1][5][1];
    success = false;
  } catch (Core::Exception& e) {
  }
  ASSERT (success);

  try {
    arX(Math::range (5), Math::range (), Math::range ());
    success = false;
  } catch (Core::Exception& e) {
  }
  ASSERT (success);

  try {
    arX(Math::range (), Math::range (4, 2), Math::range ());
    success = false;
  } catch (Core::Exception& e) {
  }
  ASSERT (success);

  try {
    arX(Math::range (), Math::range (100, std::numeric_limits<size_t>::max ()), Math::range ());
    success = false;
  } catch (Core::Exception& e) {
  }
  ASSERT (success);

  *arX.view ().pointer (1, 2, 3) = 4;
  EPRINTVALS (*arX.view ().pointer (1, 2, 3));
  EPRINTVALS (*arXC.pointer (1, 2, 3));
  (*arX.pointer (1, 2, 3))++;
  EPRINTVALS (*arX.pointer (1, 2, 3));
  //EPRINTVALS (*arX.view ().pointer (1, 2, Math::range ()));
  
#ifndef ARRAY_OMIT_ONEDIM_CTOR_OVERLOADS
  Math::Array<int, 1> ar1 (5);
  ASSERT (ar1.upperBound<0> () == 5);
  ASSERT (ar1.size<0> () == 5);
#endif

  try {
    Math::Array<int, 3> arOverflow (1000000000, 1000000000, 1000000000);
    success = false;
  } catch (Core::Exception& e) {
  }
  ASSERT (success);

  try {
    Math::Array<int, 3> arOverflow (4000000000, 4000000000, 1);
    success = false;
  } catch (Core::Exception& e) {
  }
  ASSERT (success);

  BOOST_STATIC_ASSERT (sizeof (Math::ArrayView<int, 5>) == (1 + 2 * 5) * sizeof (void*));
  BOOST_STATIC_ASSERT (sizeof (Math::ArrayView<const int, 5>) == (1 + 2 * 5) * sizeof (void*));

  Math::Array<int, 3> test1 (3, 4, 5);
  for (int i = 0; i < 3*4*5; i++)
    test1.data ()[i] = i + 1;
  std::size_t lowerBounds[] = { 5, 6, 7 };
  Math::ArrayView<int, 3, true> test2 = test1.view ();
  Math::ArrayView<int, 3, true> test3 = changeLowerBounds (test2, lowerBounds);
  ASSERT (test2.lowerBounds ()[0] == 0);
  ASSERT (test2.lowerBounds ()[1] == 0);
  ASSERT (test2.lowerBounds ()[2] == 0);
  ASSERT (test3.lowerBounds ()[0] == 5);
  ASSERT (test3.lowerBounds ()[1] == 6);
  ASSERT (test3.lowerBounds ()[2] == 7);

  ASSERT (test2[0][0][0] == 1);
  ASSERT (test3[5][6][7] == 1);
  ASSERT (test2[1][1][1] == 17);
  ASSERT (test3[6][7][8] == 17);

  Math::ArrayView<int, 2, true> test4 = test3[6];
  //EPRINTVALS (test3.lowerBound<1> (), test4.lowerBound<0> ());
  ASSERT (test3.lowerBound<1> () == test4.lowerBound<0> ());
  ASSERT (test3.lowerBound<2> () == test4.lowerBound<1> ());
  ASSERT (test3.upperBound<1> () == test4.upperBound<0> ());
  ASSERT (test3.upperBound<2> () == test4.upperBound<1> ());
  ASSERT (test3.strideBytes<1> () == test4.strideBytes<0> ());
  ASSERT (test3.strideBytes<2> () == test4.strideBytes<1> ());

  Math::ArrayView<int, 2, true> test5 = test3(Math::range (), 6, Math::range ());
  //EPRINTVALS (test3.lowerBound<0> (), test5.lowerBound<0> ());
  ASSERT (test3.lowerBound<0> () == test5.lowerBound<0> ());
  ASSERT (test3.lowerBound<2> () == test5.lowerBound<1> ());
  ASSERT (test3.upperBound<0> () == test5.upperBound<0> ());
  ASSERT (test3.upperBound<2> () == test5.upperBound<1> ());
  ASSERT (test3.strideBytes<0> () == test5.strideBytes<0> ());
  ASSERT (test3.strideBytes<2> () == test5.strideBytes<1> ());

  // Math::ArrayView<int, 2> test6 = test3(Math::range (), 6, Math::range ()); // Error

  Math::ArrayView<int, 3> test7 = test3(Math::range (5, 3), Math::range (6, 4), Math::range (7));
  //EPRINTVALS (test7.lowerBound<0> ());
  ASSERT (0 == test7.lowerBound<0> ());
  ASSERT (0 == test7.lowerBound<1> ());
  ASSERT (0 == test7.lowerBound<2> ());
  ASSERT (3 == test7.upperBound<0> ());
  ASSERT (4 == test7.upperBound<1> ());
  ASSERT (5 == test7.upperBound<2> ());
  ASSERT (test3.strideBytes<0> () == test7.strideBytes<0> ());
  ASSERT (test3.strideBytes<1> () == test7.strideBytes<1> ());
  ASSERT (test3.strideBytes<2> () == test7.strideBytes<2> ());

  try {
    test3[4][6][7];
    success = false;
  } catch (Core::Exception& e) {
  }
  ASSERT (success);

  try {
    EPRINTVALS (test3.lowerBound<2> ());
    test3[5][6][6];
    EPRINTVALS (test3.lowerBound<2> ());
    success = false;
  } catch (Core::Exception& e) {
  }
  ASSERT (success);

  try {
    test3[8][6][7];
    success = false;
  } catch (Core::Exception& e) {
  }
  ASSERT (success);

  Math::Array<int, 3> testN (3, 4, 5);
  for (int i = 0; i < 3*4*5; i++)
    testN.data ()[i] = i + 5;
  Math::ArrayView<int, 3, true> testN2 = changeLowerBounds (testN.view (), lowerBounds);
  test3.assign (testN2);
  ASSERT (test3[7][9][10] == testN2[7][9][10]);

  test3.setTo (42);
  ASSERT (test3[7][9][10] == 42);

#ifdef ARRAY_OMIT_ONEDIM_CTOR_OVERLOADS
  return 0;
#else
  return main2 ();
#endif
}
