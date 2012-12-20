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

#include "DataFilesDDAUtil.hpp"

#include <DDA/DipoleGeometry.hpp>
#include <DDA/DDAParams.hpp>

namespace DDA {
  namespace DataFiles {
    template <typename ftype>
    boost::shared_ptr<EMSim::DataFiles::Parameters<EMSim::DataFiles::DDAParameters> > createParametersDDA (const DDAParams<ftype>& ddaParams) {
      boost::shared_ptr<EMSim::DataFiles::Parameters<EMSim::DataFiles::DDAParameters> > par = boost::make_shared<EMSim::DataFiles::Parameters<EMSim::DataFiles::DDAParameters> > ();

      par->Method = "DDA";

      //par->MethodParameters.PolarizabilityType = ...;
      par->MethodParameters.Gamma = ddaParams.gamma ();
      par->MethodParameters.Frequency = ddaParams.frequency ();

      par->GeometryString = ddaParams.geometryString ();

      //par->CmdLine = ...;

      par->RefractiveIndexMedium = Math::DiagMatrix3<ldouble> (1.0);

      //par->PropagationVector = ...;
      //par->BeamString = ...;
      //par->Polarizations = ...;

      return par;
    }

    boost::shared_ptr<EMSim::DataFiles::DDADipoleListGeometry> createDDADipoleListGeometry (const DipoleGeometry& dipoleGeometry, bool full) {
      boost::shared_ptr<EMSim::DataFiles::DDADipoleListGeometry> ret = boost::make_shared<EMSim::DataFiles::DDADipoleListGeometry> ();

      ret->Type = "DDADipoleList";

      ret->GridSpacing = Math::DiagMatrix3<ldouble> (dipoleGeometry.gridUnit ());
      ret->Size = Math::Vector3<uint32_t> (dipoleGeometry.box ().x () (), dipoleGeometry.box ().y () (), dipoleGeometry.box ().z () ());
      ret->GridOrigin = dipoleGeometry.origin ();
      ret->Orientation = dipoleGeometry.orientation ().quaternion ();

      ret->RefractiveIndices.resize (dipoleGeometry.materials ().size ());
      for (size_t i = 0; i < dipoleGeometry.materials ().size (); i++)
        ret->RefractiveIndices[i] = dipoleGeometry.materials ()[i];

      if (full) {
        ret->DipolePositions = boost::make_shared<std::vector<Math::Vector3<uint32_t> > > (dipoleGeometry.positions ());
        ret->DipoleMaterialIndices = boost::make_shared<std::vector<uint8_t> > (dipoleGeometry.materialIndices ());
      }

      if (dipoleGeometry.periodicityDimension () == 0) {
        ret->Periodicity.resize (0);
      } else if (dipoleGeometry.periodicityDimension () == 1) {
        ret->Periodicity.resize (1);
        ret->Periodicity[0] = dipoleGeometry.periodicity1 ();
      } else if (dipoleGeometry.periodicityDimension () == 2) {
        ret->Periodicity.resize (2);
        ret->Periodicity[0] = dipoleGeometry.periodicity1 ();
        ret->Periodicity[1] = dipoleGeometry.periodicity2 ();
      } else {
        ABORT ();
      }

      return ret;
    }

    boost::shared_ptr<EMSim::DataFiles::DDADipoleListGeometryFile> createDDADipoleListGeometryFile (const DipoleGeometry& dipoleGeometry) {
      boost::shared_ptr<EMSim::DataFiles::DDADipoleListGeometryFile> file = boost::make_shared<EMSim::DataFiles::DDADipoleListGeometryFile> ();
      file->Type = "Geometry";
      file->Geometry = DataFiles::createDDADipoleListGeometry (dipoleGeometry, true);
      return file;
    }

    template <typename ftype>
    boost::shared_ptr<EMSim::DataFiles::DDAField<ftype> > createDDAField (const DDAParams<ftype>& ddaParams, const std::string& fieldName, uint32_t beamPolarization, const std::vector<std::complex<ftype> >& data) {
      ASSERT (data.size () == ddaParams.vecSize ());

      boost::shared_ptr<EMSim::DataFiles::DDAField<ftype> > ret = boost::make_shared<EMSim::DataFiles::DDAField<ftype> > ();
      ret->FieldName = fieldName;
      ret->BeamPolarization = beamPolarization;

      ret->Data.resize (ddaParams.nvCount ());
      for (size_t i = 0; i < ddaParams.nvCount (); i++) {
        ret->Data[i] = (Math::Vector3<std::complex<ftype> >) ddaParams.get (data, i);
      }

      return ret;
    }

    template <typename ftype>
    boost::shared_ptr<EMSim::DataFiles::DDAFieldFile<ftype> > createDDAFieldFile (const DDAParams<ftype>& ddaParams, boost::shared_ptr<EMSim::DataFiles::Parameters<EMSim::DataFiles::DDAParameters> > parameters, const std::string& fieldName, uint32_t beamPolarization, const std::vector<std::complex<ftype> >& data) {
      boost::shared_ptr<EMSim::DataFiles::DDAFieldFile<ftype> > ret = boost::make_shared<EMSim::DataFiles::DDAFieldFile<ftype> > ();
      ret->Type = "DDAField";
      ret->Parameters = parameters;
      ret->Geometry = createDDADipoleListGeometry (ddaParams.dipoleGeometry (), true);
      ret->Field = createDDAField<ftype> (ddaParams, fieldName, beamPolarization, data);
      return ret;
    }


#define TEMPL(ftype, IGNORE) template boost::shared_ptr<EMSim::DataFiles::Parameters<EMSim::DataFiles::DDAParameters> > createParametersDDA (const DDAParams<ftype>& ddaParams);
    CALL_MACRO_FOR_DEFAULT_FP_TYPES(TEMPL, IGNORE)
#undef TEMPL
#define TEMPL(ftype, IGNORE) template boost::shared_ptr<EMSim::DataFiles::DDAField<ftype> > createDDAField (const DDAParams<ftype>& ddaParams, const std::string& fieldName, uint32_t beamPolarization, const std::vector<std::complex<ftype> >& data);
    CALL_MACRO_FOR_DEFAULT_FP_TYPES(TEMPL, IGNORE)
#undef TEMPL
#define TEMPL(ftype, IGNORE) template boost::shared_ptr<EMSim::DataFiles::DDAFieldFile<ftype> > createDDAFieldFile (const DDAParams<ftype>& ddaParams, boost::shared_ptr<EMSim::DataFiles::Parameters<EMSim::DataFiles::DDAParameters> > parameters, const std::string& fieldName, uint32_t beamPolarization, const std::vector<std::complex<ftype> >& data);
    CALL_MACRO_FOR_DEFAULT_FP_TYPES(TEMPL, IGNORE)
#undef TEMPL
  }
}
