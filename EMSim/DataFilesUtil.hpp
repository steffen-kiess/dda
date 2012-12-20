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

#ifndef EMSIM_DATAFILESUTIL_HPP_INCLUDED
#define EMSIM_DATAFILESUTIL_HPP_INCLUDED

// Methods for creating EMSim::DataFiles structures

#include <EMSim/DataFiles.hpp>
#include <EMSim/Forward.hpp>

namespace EMSim {
  namespace DataFiles {
    boost::shared_ptr<Parameters<MieParameters> > createParametersMie (ldouble frequency, const std::string& geometryString);

    boost::shared_ptr<CrossSection> createCrossSection (const EMSim::CrossSection& crossSection);
    boost::shared_ptr<CrossSectionFile> createCrossSectionFile (uint32_t beamPolarization, const EMSim::CrossSection& crossSection);
  }
}

#endif // !EMSIM_DATAFILESUTIL_HPP_INCLUDED
