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

#include "DataFiles.hpp"

#include <Core/OStream.hpp>
#include <Core/BoostFilesystem.hpp>

#include <sstream>

namespace EMSim {
  namespace DataFiles {
    void CrossSectionFile::writeTxt (const Core::OStream& out) const {
      std::stringstream str;
      str.precision (10);
      str << "Cext = " << CrossSection->Cext << std::endl;
      str << "Qext = " << CrossSection->Qext << std::endl;
      str << "Cabs = " << CrossSection->Cabs << std::endl;
      str << "Qabs = " << CrossSection->Qabs << std::endl;
      str << "Csca = " << CrossSection->Csca << std::endl;
      str << "Qsca = " << CrossSection->Qsca << std::endl;
      out << str.str ();
    }

    void CrossSectionFile::write (const boost::filesystem::path& basename, boost::optional<std::string> txtExt, boost::optional<std::string> hdf5Ext) const {
      if (hdf5Ext)
        HDF5::matlabSerialize (basename.parent_path () / (basename.BOOST_FILENAME_STRING + *hdf5Ext), *this);

      if (txtExt)
        writeTxt (Core::OStream::open (basename.parent_path () / (basename.BOOST_FILENAME_STRING + *txtExt)));
    }
  }
}
