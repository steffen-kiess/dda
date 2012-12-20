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

#include "Load.hpp"

#include <Core/BoostFilesystem.hpp>

#include <HDF5/File.hpp>

#include <EMSim/DataFilesDDA.hpp>

#include <DDA/DipoleGeometry.hpp>
#include <DDA/DDAParams.hpp>

#include <boost/filesystem.hpp>

namespace DDA {
  void loadDipoleGeometry (DipoleGeometry& dipoleGeometry, const Core::IStream& infile) {
    ASSERT (!infile->fail ());
    dipoleGeometry.clear ();
    std::string data;
    while (!infile->eof ()) {
      while (infile->peek () == '#' || infile->peek () == 'N')
        while (infile.good () && infile->get () != '\n')
          ;
      if (infile->eof ()) {
        dipoleGeometry.moveToCenter ();
        return;
      }
      infile.assertGood ();
      uint32_t x, y, z, m;
      *infile >> x;
      if (infile->eof ()) {
        dipoleGeometry.moveToCenter ();
        return;
      }
      infile >> y >> z;
      int c = infile->peek ();
      if (c == 10 || c == 13) {
        m = 1;
      } else {
        infile >> m;
      }
      ASSERT (!infile->fail ());
      ASSERT (m > 0 && m <= 256);
      dipoleGeometry.addDipole (x, y, z, (uint8_t) (m - 1));
      //Core::OStream::getStdout () << "X" << x << " " << y << " " << z << " " << (int) m << std::endl;
    }
    dipoleGeometry.moveToCenter ();
  }

  void loadDipoleGeometry (DipoleGeometry& dipoleGeometry, const boost::filesystem::path& filename) {
    loadDipoleGeometry (dipoleGeometry, Core::IStream::open (filename));
  }

  template <class ftype>
  void Load<ftype>::loadFields (const Core::IStream& in, const DDAParams<ftype>& ddaParams, const std::string& name, std::vector<std::complex<ftype> >& field) {
    std::stringstream str;
    str << "#x y z |" << name << "|^2 " << name << "x.r " << name << "x.i " << name << "y.r " << name << "y.i " << name << "z.r " << name << "z.i";
    std::string s2;
    getline (*in, s2);
    in.assertGood ();
    //ASSERT (str.str () == s2);
    field.resize (ddaParams.vecSize ());
    for (uint32_t i = 0; i < ddaParams.cnvCount (); i++) {
      Math::Vector3<ftype> coord = ddaParams.dipoleGeometry ().getDipoleCoordPartRef (i);
      Math::Vector3<ftype> coord2;
      ftype normalized2;

      ftype xr, xi, yr, yi, zr, zi;
      in >> coord2.x () >> coord2.y () >> coord2.z ()
         >> normalized2
         >> xr >> xi >> yr >> yi >> zr >> zi;
      Math::Vector3<std::complex<ftype> > vec (std::complex<ftype> (xr, xi),
                                               std::complex<ftype> (yr, yi),
                                               std::complex<ftype> (zr, zi));
      ASSERT (Math::abs2 (coord2 - coord) < 1e-5);
      ftype normalized = Math::abs2 (vec);
      ASSERT (std::abs (normalized2 - normalized) < 1e-5);
      ddaParams.set (field, i, vec);
    }
    // Make sure we've reached EOF
    int i;
    *in >> i;
    ASSERT (in->eof ());
  }

  template <class ftype>
  void Load<ftype>::loadFields (const boost::filesystem::path& inputFile, const DDAParams<ftype>& ddaParams, const std::string& name, std::vector<std::complex<ftype> >& field) {
    loadFields (Core::IStream::open (inputFile), ddaParams, name, field);
  }

  template <typename ftype>
  void Load<ftype>::loadDipPol (const boost::filesystem::path& inputBasename, const DDAParams<ftype>& ddaParams, std::vector<std::complex<ftype> >& field) {
    boost::filesystem::path inputHdf5 = inputBasename.parent_path () / (inputBasename.BOOST_FILENAME_STRING + ".hdf5");
    boost::filesystem::path inputTxt = inputBasename.parent_path () / (inputBasename.BOOST_FILENAME_STRING + ".txt");

    if (!boost::filesystem::exists (inputHdf5) && boost::filesystem::exists (inputTxt)) {
      loadFields (inputTxt, ddaParams, "P", field);
    } else {
      HDF5::File file = HDF5::File::open (inputHdf5, H5F_ACC_RDONLY);
      boost::shared_ptr<EMSim::DataFiles::DDAFieldFileLight<ftype> > ptr = HDF5::matlabDeserialize<EMSim::DataFiles::DDAFieldFileLight<ftype> > (file);
      boost::shared_ptr<EMSim::DataFiles::DDADipoleListGeometryFileLight> geometry = HDF5::matlabDeserialize<EMSim::DataFiles::DDADipoleListGeometryFileLight> (file);

      ASSERT (ddaParams.cnvCount () == ptr->Field->Data.size ());
      field.resize (ddaParams.vecSize ());

      for (size_t i = 0; i < ddaParams.cnvCount (); i++) {
        if (geometry->Geometry && geometry->Geometry->DipolePositions) {
          Math::Vector3<uint32_t> coord = ddaParams.dipoleGeometry ().getGridCoordinates (i);
          Math::Vector3<uint32_t> coord2 = (*geometry->Geometry->DipolePositions)[i];
          ASSERT (coord == coord2);
        }
        ddaParams.template set<std::complex<ftype> > (field, i, ptr->Field->Data[i]);
      }
    }
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, Load)
}
