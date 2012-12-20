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

#include "DataFilesUtil.hpp"

#include <EMSim/CrossSection.hpp>

namespace EMSim {
  namespace DataFiles {
    boost::shared_ptr<Parameters<MieParameters> > createParametersMie (ldouble frequency, const std::string& geometryString) {
      boost::shared_ptr<Parameters<MieParameters> > par = boost::make_shared<Parameters<MieParameters> > ();

      par->Method = "Mie";

      par->MethodParameters.Frequency = frequency;
    
      par->GeometryString = geometryString;
    
      //par->CmdLine = ...;

      par->RefractiveIndexMedium = Math::DiagMatrix3<ldouble> (1.0);

      //par->PropagationVector = ...;
      //par->BeamString = ...;
      //par->Polarizations = ...;

      return par;
    }

    boost::shared_ptr<CrossSection> createCrossSection (const EMSim::CrossSection& crossSection) {
      boost::shared_ptr<CrossSection> ret = boost::make_shared<CrossSection> ();
      ret->Cext = crossSection.Cext;
      ret->Cabs = crossSection.Cabs;
      ret->Csca = crossSection.Csca;
      ret->Qext = crossSection.Qext;
      ret->Qabs = crossSection.Qabs;
      ret->Qsca = crossSection.Qsca;
      return ret;
    }

    boost::shared_ptr<CrossSectionFile> createCrossSectionFile (uint32_t beamPolarization, const EMSim::CrossSection& crossSection) {
      boost::shared_ptr<CrossSectionFile> ret = boost::make_shared<CrossSectionFile> ();
      ret->Type = "CrossSection";
      ret->BeamPolarization = beamPolarization;
      ret->CrossSection = createCrossSection (crossSection);
      return ret;
    }
  }
}
