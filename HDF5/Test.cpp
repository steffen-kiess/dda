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

#include <HDF5/StdVectorSerialization.hpp>
#include <HDF5/Vector3.hpp>
#include <HDF5/MatlabFull.hpp>
#include <HDF5/MultiArray.hpp>
#include <HDF5/DelayedArray.hpp>

#include <HDF5/File.hpp>
#include <HDF5/PropLists.hpp>

#include <lib/utf8cpp/utf8.h>

#include <boost/multi_array.hpp>

class CalcInfoSer {
public:
  HDF5::ObjectReference grid;
  Math::Vector3<double> prop;
  const char* beamName;
  const char* polName;

  HDF5_TYPE (CalcInfoSer)
  HDF5_ADD_MEMBER (grid)
  HDF5_ADD_MEMBER (prop)
  HDF5_ADD_MEMBER (beamName)
  HDF5_ADD_MEMBER (polName)
  HDF5_TYPE_END
};
class CalcInfo {
public:
  CalcInfo () {}
  CalcInfo (boost::shared_ptr<std::vector<std::complex<double> > > grid, Math::Vector3<double> prop, std::string beamName, std::string polName) : grid (grid), prop (prop), beamName (beamName), polName (polName), b (true), array3 (boost::make_shared<boost::multi_array<double, 3> > (boost::extents[0][0][0], boost::fortran_storage_order ())) {
    typedef boost::multi_array<double, 3> AT;
    boost::array<AT::index, 3> dim = {{5, 10, 20}};
    array3->resize (dim);
    for (int i = 0; i < 5; i++)
      for (int j = 0; j < 10; j++)
        for (int k = 0; k < 20; k++)
          (*array3)[i][j][k] = i + j + k;
    darray3.size[0] = 3;
    darray3.size[1] = 4;
    darray3.size[2] = 5;
  }

  boost::shared_ptr<std::vector<std::complex<double> > > grid;
  Math::Vector3<double> prop;
  std::string beamName;
  std::string polName;
  bool b;
  HDF5::DataSet ds;
  float f;
  boost::shared_ptr<boost::multi_array<double, 3> > array3;
  HDF5::DelayedArray<double, 3> darray3;

#define MEMBERS_CalcInfo(m)                     \
  m (grid)                                      \
  m (prop)                                      \
  m (beamName)                                  \
  m (polName)                                   \
  m (b)                                         \
  /*m (ds)*/                                    \
  m (f)                                         \
  m (array3)                                    \
  m (darray3)

  HDF5_MATLAB_DECLARE_TYPE (CalcInfo, MEMBERS_CalcInfo)

  void h5Save (HDF5::SerializationContext& context) const {
    HDF5::DataSet dataset = context.createDataSet (*this, HDF5::getH5Type<CalcInfoSer> ());
    CalcInfoSer s;
    //context.writeRef (s.grid, *grid);
    s.grid = context.get (*grid);
    s.prop = prop;
    s.beamName = beamName.c_str ();
    s.polName = polName.c_str ();
    dataset.write (&s, HDF5::getH5Type<CalcInfoSer> ());
  }

  static void h5Load (HDF5::DeserializationContext& context, const HDF5::ObjectReference& name, HDF5::DataSet& dataSet) {
    boost::shared_ptr<CalcInfo> d (new CalcInfo (boost::shared_ptr<std::vector<std::complex<double> > > (), Math::Vector3<double> (0, 0, 0), "", ""));
    context.registerValue (name, d);
    CalcInfoSer s;
    s.beamName = NULL; // DipPolCombine doesn't set these
    s.polName = NULL;
    dataSet.read (&s, HDF5::getH5Type<CalcInfoSer> ());
    d->grid = context.resolve<std::vector<std::complex<double> > > (s.grid);
    d->prop = s.prop;
    d->beamName = s.beamName ? s.beamName : "<undefined>";
    d->polName = s.polName ? s.polName : "<undefined>";
    HDF5::DataSet::vlenReclaim (&s, HDF5::getH5Type<CalcInfoSer> (), dataSet.getSpace ());
  }
};

int main () {
  {
    boost::shared_ptr <std::vector<std::complex<double> > > v (new std::vector<std::complex<double> >);
    v->push_back (10.2);
    v->push_back (std::complex<double> (3, 4));

    CalcInfo calcInfo (v, Math::Vector3<double> (3, 4, 5), "asd", "dsa");
    calcInfo.f = 3.14f;


    HDF5::serialize ("output.hdf5", "ds", calcInfo);
    HDF5::File f = HDF5::File::open ("output.hdf5", H5F_ACC_RDONLY);
    boost::shared_ptr<CalcInfo> info2 = HDF5::deserialize<CalcInfo> ("output.hdf5", "ds");
    HDF5::serialize ("output2.hdf5", "ds", *info2);

    std::vector <uint16_t> str;
    std::string s = "asdfä";
    utf8::utf8to16 (s.begin (), s.end (), std::back_inserter (str));
    //HDF5::serialize ("output3.mat", "a", str);

  
    //HDF5::matlabSerialize ("output3.mat", "ds", calcInfo);
    //calcInfo.grid = boost::shared_ptr<std::vector<std::complex<double> > > ();
    HDF5::matlabSerialize ("output3.mat", calcInfo);
    double dd[3*4*5];
    for (int i = 0; i < 3*4*5; i++)
      dd[i] = i;
    calcInfo.darray3.write (dd);

    //CalcInfo info3 = *HDF5::matlabDeserialize<CalcInfo> ("output3.mat", "ds");
    CalcInfo info3 = *HDF5::matlabDeserialize<CalcInfo> ("output3.mat");
    //CalcInfo info3 = *HDF5::matlabDeserialize<CalcInfo> ("output5.mat");
    double dd2[3*4*5];
    info3.darray3.read (dd2);
    //HDF5::matlabSerialize ("output4.mat", "ds", info3);
    HDF5::matlabSerialize ("output4.mat", info3);
    info3.darray3.write (dd2);
  }

  HDF5::DataSetCreatePropList p = HDF5::DataSetCreatePropList::create ();
  HDF5::ObjectCreatePropList p2 = p;

  if (system ("ls -l /proc/$PPID/fd/"))
    ABORT ();

  return 0;
}
