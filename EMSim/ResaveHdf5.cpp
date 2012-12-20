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

#include "ResaveHdf5.hpp"

#include <EMSim/DataFilesDDA.hpp>

#include <boost/math/constants/constants.hpp>

namespace EMSim {
  boost::shared_ptr<DataFiles::JonesFarField<double> > removeThetaLarger180 (const boost::shared_ptr<DataFiles::JonesFarField<double> >& farField) {
    size_t fcount = farField->Frequency.size ();
    size_t count = 0;
    std::vector<bool> include (farField->Theta.size ());
    for (size_t i = 0; i < include.size (); i++) {
      bool inc = farField->Theta[i] * 180 / boost::math::constants::pi<double> () <= 180.0001;
      include[i] = inc;
      if (inc)
        count++;
    }
    boost::shared_ptr<DataFiles::JonesFarField<double> > newFarField = boost::make_shared<DataFiles::JonesFarField<double> > ();
    newFarField->Theta.resize (count);
    newFarField->Phi.resize (count);
    newFarField->Frequency.resize (fcount);
    for (size_t j = 0; j < fcount; j++)
      newFarField->Frequency[j] = farField->Frequency[j];
    newFarField->Data = boost::make_shared<boost::multi_array<std::complex<double>, 4> > (boost::extents[2][2][count][fcount], boost::fortran_storage_order ());
    size_t pos = 0;
    for (size_t i = 0; i < include.size (); i++) {
      if (include[i]) {
        newFarField->Theta[pos] = farField->Theta[i];
        newFarField->Phi[pos] = farField->Phi[i];
        for (size_t j = 0; j < fcount; j++)
          for (int a = 0; a < 2; a++)
            for (int b = 0; b < 2; b++)
              (*newFarField->Data)[a][b][pos][j] = (*farField->Data)[a][b][i][j];
        pos++;
      }
    }
    return newFarField;
  }
  boost::shared_ptr<DataFiles::MuellerFarField<double> > removeThetaLarger180 (const boost::shared_ptr<DataFiles::MuellerFarField<double> >& muellerFarField) {
    size_t fcount = muellerFarField->Frequency.size ();
    size_t count = 0;
    std::vector<bool> include (muellerFarField->Theta.size ());
    for (size_t i = 0; i < include.size (); i++) {
      bool inc = muellerFarField->Theta[i] * 180 / boost::math::constants::pi<double> () <= 180.0001;
      include[i] = inc;
      if (inc)
        count++;
    }
    boost::shared_ptr<DataFiles::MuellerFarField<double> > newMuellerFarField = boost::make_shared<DataFiles::MuellerFarField<double> > ();
    newMuellerFarField->Theta.resize (count);
    newMuellerFarField->Phi.resize (count);
    newMuellerFarField->Frequency.resize (fcount);
    for (size_t j = 0; j < fcount; j++)
      newMuellerFarField->Frequency[j] = muellerFarField->Frequency[j];
    newMuellerFarField->Data = boost::make_shared<boost::multi_array<double, 4> > (boost::extents[4][4][count][fcount], boost::fortran_storage_order ());
    size_t pos = 0;
    for (size_t i = 0; i < include.size (); i++) {
      if (include[i]) {
        newMuellerFarField->Theta[pos] = muellerFarField->Theta[i];
        newMuellerFarField->Phi[pos] = muellerFarField->Phi[i];
        for (size_t j = 0; j < fcount; j++)
          for (int a = 0; a < 4; a++)
            for (int b = 0; b < 4; b++)
              (*newMuellerFarField->Data)[a][b][pos][j] = (*muellerFarField->Data)[a][b][i][j];
        pos++;
      }
    }
    return newMuellerFarField;
  }

  void resaveHdf5 (const boost::filesystem::path& input, const boost::filesystem::path& output, const boost::optional<std::string>& typeOverwrite, bool thetaMax180) {
    HDF5::File inputFile = HDF5::File::open (input, H5F_ACC_RDONLY);
    boost::shared_ptr<DataFiles::File> fileInfo;
    if (typeOverwrite) {
      fileInfo = boost::make_shared<DataFiles::File> ();
      fileInfo->Type = *typeOverwrite;
    } else {
      fileInfo = HDF5::matlabDeserialize<DataFiles::File> (inputFile);
    }
    //Core::OStream::getStderr () << fileInfo->Type << std::endl;
    if (fileInfo->Type == "JonesFarField") {
      //boost::shared_ptr<DataFiles::JonesFarFieldFile<double> > data = HDF5::matlabDeserialize<DataFiles::JonesFarFieldFile<double> > (inputFile);
      //boost::shared_ptr<DataFiles::JonesFarField<double> > farField = data.FarField;
      boost::shared_ptr<DataFiles::JonesFarField<double> > farField = HDF5::matlabDeserialize<DataFiles::JonesFarField<double> > (inputFile, "JonesFarField");
      if (thetaMax180)
        farField = removeThetaLarger180 (farField);

      HDF5::File outputFile = HDF5::createMatlabFile (output);
      HDF5::matlabSerialize (outputFile, "JonesFarField", farField);
      if (inputFile.rootGroup ().exists ("Parameters"))
        HDF5::Group::copyObject (inputFile.rootGroup (), "Parameters", outputFile.rootGroup (), "Parameters");
      if (inputFile.rootGroup ().exists ("Geometry"))
        HDF5::Group::copyObject (inputFile.rootGroup (), "Geometry", outputFile.rootGroup (), "Geometry");
      HDF5::matlabSerialize (outputFile, "Type", "JonesFarField");
    } else if (fileInfo->Type == "MuellerFarField") {
      //boost::shared_ptr<DataFiles::MuellerFarFieldFile<double> > data = HDF5::matlabDeserialize<DataFiles::MuellerFarFieldFile<double> > (inputFile);
      //boost::shared_ptr<DataFiles::MuellerFarField<double> > muellerFarField = data.MuellerFarField;
      boost::shared_ptr<DataFiles::MuellerFarField<double> > muellerFarField = HDF5::matlabDeserialize<DataFiles::MuellerFarField<double> > (inputFile, "MuellerFarField");
      if (thetaMax180) 
        muellerFarField = removeThetaLarger180 (muellerFarField);

      HDF5::File outputFile = HDF5::createMatlabFile (output);
      HDF5::matlabSerialize (outputFile, "MuellerFarField", muellerFarField);
      if (inputFile.rootGroup ().exists ("Parameters"))
        HDF5::Group::copyObject (inputFile.rootGroup (), "Parameters", outputFile.rootGroup (), "Parameters");
      if (inputFile.rootGroup ().exists ("Geometry"))
        HDF5::Group::copyObject (inputFile.rootGroup (), "Geometry", outputFile.rootGroup (), "Geometry");
      HDF5::matlabSerialize (outputFile, "Type", "MuellerFarField");
    } else if (fileInfo->Type == "DDAField") {
      HDF5::matlabSerialize (output, *HDF5::matlabDeserialize<DataFiles::DDAFieldFile<double> > (inputFile));
    } else if (fileInfo->Type == "Geometry") {
      HDF5::matlabSerialize (output, *HDF5::matlabDeserialize<DataFiles::DDADipoleListGeometryFile> (inputFile));
    } else if (fileInfo->Type == "CrossSection") {
      HDF5::matlabSerialize (output, *HDF5::matlabDeserialize<DataFiles::CrossSectionFile> (inputFile));
    } else {
      ABORT_MSG ("Unknown file type '" + fileInfo->Type + "'");
    }
  }
}
