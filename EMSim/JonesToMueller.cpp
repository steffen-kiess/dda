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

#include "JonesToMueller.hpp"

#include <HDF5/File.hpp>

#include <EMSim/MuellerCalculus.hpp>
#include <EMSim/DataFiles.hpp>
#include <EMSim/ResaveHdf5.hpp>

namespace EMSim {
  void jonesToMueller (const boost::filesystem::path& inputFileName, const boost::filesystem::path& outputFileName, bool thetaMax180) {
    HDF5::File inputFile = HDF5::File::open (inputFileName, H5F_ACC_RDONLY);
    boost::shared_ptr<DataFiles::JonesFarField<double> > farField = HDF5::matlabDeserialize<DataFiles::JonesFarField<double> > (inputFile, "JonesFarField");
    if (thetaMax180)
      farField = removeThetaLarger180 (farField);
    boost::shared_ptr<DataFiles::MuellerFarField<double> > muellerFarField = MuellerCalculus<double>::computeMuellerFarField (farField);

    HDF5::File outputFile = HDF5::createMatlabFile (outputFileName);
    HDF5::matlabSerialize (outputFile, "Type", "MuellerFarField");
    if (inputFile.rootGroup ().exists ("Parameters"))
      HDF5::Group::copyObject (inputFile.rootGroup (), "Parameters", outputFile.rootGroup (), "Parameters");
    if (inputFile.rootGroup ().exists ("Geometry"))
      HDF5::Group::copyObject (inputFile.rootGroup (), "Geometry", outputFile.rootGroup (), "Geometry");
    HDF5::matlabSerialize (outputFile, "MuellerFarField", muellerFarField);
  }
}
