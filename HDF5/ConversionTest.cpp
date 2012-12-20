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

// Tests for HDF5 conversions, in particular for real->complex and complex->real
// conversions

#include <Core/OStream.hpp>

#include <Math/Float.hpp>

#include <HDF5/Type.hpp>
#include <HDF5/ComplexConversion.hpp>

#include <complex>
#include <vector>

#include <boost/type_traits/alignment_of.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/scoped_array.hpp>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

template <typename To, typename From>
To convert (const From& from) {
  typedef boost::aligned_storage<MAX(sizeof (To), sizeof (From)), MAX(boost::alignment_of<To>::value, boost::alignment_of<From>::value)> Buffer;

  Buffer val;
  *(From*)&val = from;

  Buffer bkg;

  HDF5::Exception::check ("H5Tconvert", H5Tconvert (HDF5::getH5Type<From> ().handle (), HDF5::getH5Type<To> ().handle (), 1, &val, &bkg, H5P_DEFAULT));
  
  //return val.toVal;
  return *(To*)&val;
}

template <typename T>
std::ostream& operator<< (std::ostream& str, const std::vector<T>& v) {
  str << "{";
  for (size_t i = 0; i < v.size (); i++) {
    if (i)
      str << ", ";
    str << v[i];
  }
  str << "}";
  return str;
}

template <typename To, typename From>
std::vector<To> convert (const std::vector<From>& from) {
  typedef boost::aligned_storage<MAX(sizeof (To), sizeof (From)), MAX(boost::alignment_of<To>::value, boost::alignment_of<From>::value)> Buffer;

  boost::scoped_array<Buffer> val (new Buffer[from.size ()]);
  for (size_t i = 0; i < from.size (); i++)
    ((From*)&val[0])[i] = from[i];

  boost::scoped_array<Buffer> bkg (new Buffer[from.size ()]);

  HDF5::Exception::check ("H5Tconvert", H5Tconvert (HDF5::getH5Type<From> ().handle (), HDF5::getH5Type<To> ().handle (), from.size (), val.get (), bkg.get (), H5P_DEFAULT));
  
  std::vector<To> result (from.size ());
  for (size_t i = 0; i < from.size (); i++)
    result[i] = ((To*)val.get ())[i];
  return result;
}

template <typename T>
std::vector<T> makev () {
  return std::vector<T> ();
}
template <typename T>
std::vector<T> makev (T v1) {
  std::vector<T> ret;
  ret.push_back (v1);
  return ret;
}
template <typename T>
std::vector<T> makev (T v1, T v2) {
  std::vector<T> ret;
  ret.push_back (v1);
  ret.push_back (v2);
  return ret;
}
template <typename T>
std::vector<T> makev (T v1, T v2, T v3) {
  std::vector<T> ret;
  ret.push_back (v1);
  ret.push_back (v2);
  ret.push_back (v3);
  return ret;
}

struct ErrorData {
  hid_t cls;
  hid_t major;
  hid_t minor;
};
static ErrorData createErrorData () {
  ErrorData e;

  e.cls = HDF5::Exception::check ("H5Eregister_class", H5Eregister_class ("HDF5", "ConversionTest", "0.0"));
  e.major = HDF5::Exception::check ("H5Ecreate_msg", H5Ecreate_msg (e.cls, H5E_MAJOR, "major message"));
  e.minor = HDF5::Exception::check ("H5Ecreate_msg", H5Ecreate_msg (e.cls, H5E_MINOR, "minor message"));

  return e;
}
UNUSED static const ErrorData& getErrorData () {
  static ErrorData e = createErrorData ();
  return e;
}

template <typename T>
std::string toString (const T& val) {
  std::stringstream str;
  str << val;
  return str.str ();
}

#define COMP(expr, str)                                                 \
  do {                                                                  \
    BOOST_AUTO (val, expr);                                             \
    std::string s = toString (val);                                     \
    if (s != (str)) {                                                   \
      ABORT_MSG ("Incorrect result: For expression `" #expr "' expected `" + std::string (str) + "', got `" + s + "'"); \
    }                                                                   \
  } while (0)


int main () {
  //HDF5::registerComplexConversion ();
  //HDF5::registerComplexConversion ();

  COMP (convert<uint64_t> (0.0), "0");
  COMP (convert<uint64_t> (1.0), "1");
  COMP (convert<uint64_t> (0.9), "0");
  COMP (convert<uint64_t> (1.1), "1");

  COMP (convert<std::complex<double> > (1.1), "(1.1,0)");

  COMP (convert<cldouble > (1.1l), "(1.1,0)");
  COMP (convert<cldouble > (1.1), "(1.1,0)");
  COMP (convert<cldouble > (1.1f), "(1.1,0)");

  COMP (convert<std::complex<double> > (1.1l), "(1.1,0)");
  COMP (convert<std::complex<double> > (1.1), "(1.1,0)");
  COMP (convert<std::complex<double> > (1.1f), "(1.1,0)");

  COMP (convert<std::complex<float> > (1.1l), "(1.1,0)");
  COMP (convert<std::complex<float> > (1.1), "(1.1,0)");
  COMP (convert<std::complex<float> > (1.1f), "(1.1,0)");

  COMP (convert<std::complex<float> > (5), "(5,0)");

  COMP (convert<std::complex<int> > (5), "(5,0)");
  COMP (convert<std::complex<int16_t> > (5.0l), "(5,0)");

  COMP (convert<std::complex<double> > (std::complex<float> (4)), "(4,0)");
  COMP (convert<std::complex<float> > (std::complex<double> (4)), "(4,0)");

  COMP (convert<float> (makev(4.0, 5.0)), "{4, 5}");
  COMP (convert<int16_t> (makev(4.0l, 5.0l)), "{4, 5}");
  COMP (convert<ldouble> (makev<uint8_t>(4, 5)), "{4, 5}");

  COMP (convert<std::complex<float> > (makev(4.0l, 5.0l)), "{(4,0), (5,0)}");
  COMP (convert<std::complex<int16_t> > (makev(4.0l, 5.0l)), "{(4,0), (5,0)}");
  COMP (convert<cldouble > (makev<uint8_t>(4, 5)), "{(4,0), (5,0)}");

  COMP (convert<float> (std::complex<float> (4.1f)), "4.1");
  COMP (convert<float> (std::complex<double> (4.1)), "4.1");
  COMP (convert<float> (cldouble (4.1l)), "4.1");
  COMP (convert<double> (std::complex<float> (4.1f)), "4.1");
  COMP (convert<double> (std::complex<double> (4.1)), "4.1");
  COMP (convert<double> (cldouble (4.1l)), "4.1");
  COMP (convert<ldouble> (std::complex<float> (4.1f)), "4.1");
  COMP (convert<ldouble> (std::complex<double> (4.1)), "4.1");
  COMP (convert<ldouble> (cldouble (4.1l)), "4.1");

  COMP (convert<float> (makev (std::complex<float> (4.1f), std::complex<float> (4.2f))), "{4.1, 4.2}");
  COMP (convert<float> (makev (std::complex<double> (4.1), std::complex<double> (4.2))), "{4.1, 4.2}");
  COMP (convert<float> (makev (cldouble (4.1l), cldouble (4.2l))), "{4.1, 4.2}");
  COMP (convert<double> (makev (std::complex<float> (4.1f), std::complex<float> (4.2f))), "{4.1, 4.2}");
  COMP (convert<double> (makev (std::complex<double> (4.1), std::complex<double> (4.2))), "{4.1, 4.2}");
  COMP (convert<double> (makev (cldouble (4.1l), cldouble (4.2l))), "{4.1, 4.2}");
  COMP (convert<ldouble> (makev (std::complex<float> (4.1f), std::complex<float> (4.2f))), "{4.1, 4.2}");
  COMP (convert<ldouble> (makev (std::complex<double> (4.1), std::complex<double> (4.2))), "{4.1, 4.2}");
  COMP (convert<ldouble> (makev (cldouble (4.1l), cldouble (4.2l))), "{4.1, 4.2}");

  COMP (convert<ldouble> (makev (std::complex<uint8_t> (4), std::complex<uint8_t> (5))), "{4, 5}");
  COMP (convert<uint16_t> (makev (cldouble (4.0l), cldouble (5.0l))), "{4, 5}");

  HDF5::Exception::check ("H5Eset_auto2", H5Eset_auto2 (H5E_DEFAULT, NULL, NULL));
  try {
    COMP (convert<ldouble> (makev (std::complex<uint8_t> (4, 40), std::complex<uint8_t> (5, 50))), "XXX");
  } catch (HDF5::Exception& e) {
    //EPRINTVAL (e.message ());
  }
  try {
    COMP (convert<uint16_t> (makev (cldouble (4, 40), cldouble (5, 50))), "XXX");
  } catch (HDF5::Exception& e) {
    //EPRINTVAL (e.message ());
  }

  return 0;
}
