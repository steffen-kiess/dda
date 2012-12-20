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

#include "Hdf5ToText.hpp"

#include <Core/Assert.hpp>
#include <Core/BoostFilesystem.hpp>
#include <Core/OStream.hpp>

#include <HDF5/File.hpp>

#include <EMSim/MuellerCalculus.hpp>
#include <EMSim/JonesCalculus.hpp>
#include <EMSim/DataFiles.hpp>
#include <EMSim/DataFilesDDA.hpp>
#include <EMSim/ResaveHdf5.hpp>

#include <sstream>
#include <vector>

#include <stdint.h>

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/math/constants/constants.hpp>

namespace EMSim {
  namespace {
    struct GetStreamVisitor : public boost::static_visitor<Core::OStream> {
      Core::OStream operator() (const Core::OStream& stream) const {
        return stream;
      }
    
      Core::OStream operator() (const boost::filesystem::path& path) const {
        return Core::OStream::open (path);
      }
    };
    Core::OStream getStream (const StreamOrPathOutput& output) {
      return boost::apply_visitor (GetStreamVisitor (), output);
    }
  }

  void hdf5ToText (const boost::filesystem::path& input, const StreamOrPathOutput& output, const boost::optional<std::string>& typeOverwrite, bool thetaMax180, bool noPhi) {
    HDF5::File file = HDF5::File::open (input, H5F_ACC_RDONLY);
    boost::shared_ptr<DataFiles::File> fileInfo;
    if (typeOverwrite) {
      fileInfo = boost::make_shared<DataFiles::File> ();
      fileInfo->Type = *typeOverwrite;
    } else {
      fileInfo = HDF5::matlabDeserialize<DataFiles::File> (file);
    }
    //Core::OStream::getStderr () << fileInfo->Type << std::endl;
    if (fileInfo->Type == "JonesFarField") {
      //boost::shared_ptr<DataFiles::JonesFarFieldFile<double> > data = HDF5::matlabDeserialize<DataFiles::JonesFarFieldFile<double> > (file);
      //boost::shared_ptr<DataFiles::JonesFarField<double> > farField = data.FarField;
      boost::shared_ptr<DataFiles::JonesFarField<double> > farField = HDF5::matlabDeserialize<DataFiles::JonesFarField<double> > (file, "JonesFarField");
      if (thetaMax180)
        farField = removeThetaLarger180 (farField);
      JonesCalculus<double>::storeTxt (getStream (output), farField, !noPhi);
    } else if (fileInfo->Type == "MuellerFarField") {
      //boost::shared_ptr<DataFiles::MuellerFarFieldFile<double> > data = HDF5::matlabDeserialize<DataFiles::MuellerFarFieldFile<double> > (file);
      //boost::shared_ptr<DataFiles::MuellerFarField<double> > muellerFarField = data.MuellerFarField;
      boost::shared_ptr<DataFiles::MuellerFarField<double> > muellerFarField = HDF5::matlabDeserialize<DataFiles::MuellerFarField<double> > (file, "MuellerFarField");
      if (thetaMax180)
        muellerFarField = removeThetaLarger180 (muellerFarField);
      MuellerCalculus<double>::storeTxt (getStream (output), muellerFarField, noPhi);
    } else if (fileInfo->Type == "DDAField") {
      HDF5::matlabDeserialize<DataFiles::DDAFieldFile<double> > (file)->writeTxt (getStream (output));
    } else if (fileInfo->Type == "Geometry") {
      HDF5::matlabDeserialize<DataFiles::DDADipoleListGeometryFile> (file)->writeTxt (getStream (output));
    } else if (fileInfo->Type == "CrossSection") {
      HDF5::matlabDeserialize<DataFiles::CrossSectionFile> (file)->writeTxt (getStream (output));
    } else {
      ABORT_MSG ("Unknown file type '" + fileInfo->Type + "'");
    }
  }
}
