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

#ifndef DDA_DATAFILESDDAUTIL_HPP_INCLUDED
#define DDA_DATAFILESDDAUTIL_HPP_INCLUDED

// Methods for creating EMSim::DataFiles structures

#include <EMSim/DataFilesDDA.hpp>

#include <DDA/Forward.hpp>

namespace DDA {
  namespace DataFiles {
    template <typename ftype>
    boost::shared_ptr<EMSim::DataFiles::Parameters<EMSim::DataFiles::DDAParameters> > createParametersDDA (const DDAParams<ftype>& ddaParams);

    boost::shared_ptr<EMSim::DataFiles::DDADipoleListGeometry> createDDADipoleListGeometry (const DipoleGeometry& dipoleGeometry, bool full);
    boost::shared_ptr<EMSim::DataFiles::DDADipoleListGeometryFile> createDDADipoleListGeometryFile (const DipoleGeometry& dipoleGeometry);

    template <typename ftype>
    boost::shared_ptr<EMSim::DataFiles::DDAField<ftype> > createDDAField (const DDAParams<ftype>& ddaParams, const std::string& fieldName, uint32_t beamPolarization, const std::vector<std::complex<ftype> >& data);

    template <typename ftype>
    boost::shared_ptr<EMSim::DataFiles::DDAFieldFile<ftype> > createDDAFieldFile (const DDAParams<ftype>& ddaParams, boost::shared_ptr<EMSim::DataFiles::Parameters<EMSim::DataFiles::DDAParameters> > parameters, const std::string& fieldName, uint32_t beamPolarization, const std::vector<std::complex<ftype> >& data);
  }
}

#endif // !DDA_DATAFILESDDAUTIL_HPP_INCLUDED
