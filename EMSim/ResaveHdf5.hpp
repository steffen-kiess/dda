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

#ifndef EMSIM_RESAVEHDF5_HPP_INCLUDED
#define EMSIM_RESAVEHDF5_HPP_INCLUDED

// Method to load and resave a HDF5 file

#include <EMSim/DataFiles.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

namespace EMSim {
  boost::shared_ptr<DataFiles::JonesFarField<double> > removeThetaLarger180 (const boost::shared_ptr<DataFiles::JonesFarField<double> >& farField);
  boost::shared_ptr<DataFiles::MuellerFarField<double> > removeThetaLarger180 (const boost::shared_ptr<DataFiles::MuellerFarField<double> >& muellerFarField);

  void resaveHdf5 (const boost::filesystem::path& input, const boost::filesystem::path& output, const boost::optional<std::string>& typeOverwrite, bool thetaMax180);
}

#endif // !EMSIM_RESAVEHDF5_HPP_INCLUDED
