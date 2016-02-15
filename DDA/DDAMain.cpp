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

#include "DDAMain.h"

// The main method of the DDA implementation
//
// This file contains the code which will call all the individual components
// (like dmatrix calculation, iterative solver and far field calculation).

#include <Core/Profiling.hpp>
#include <Core/OStream.hpp>
#include <Core/IStream.hpp>
#include <Core/StringUtil.hpp>
#include <Core/Error.hpp>
#include <Core/File.hpp>
#include <Core/HelpResultException.hpp>

#include <OpenCL/Context.hpp>

#include <LinAlg/FFTWPlan.hpp>

#include <Math/DiagMatrix3.hpp>
#include <Math/DiagMatrix3IOS.hpp>
#include <Math/Math.hpp>

#include <EMSim/CrossSection.hpp>
#include <EMSim/Length.hpp>
#include <EMSim/Mie.hpp>
#include <EMSim/OutputDirectory.hpp>
#include <EMSim/DataFilesUtil.hpp>

#include <DDA/MatVecCpu.hpp>
#include <DDA/MatVecGpu.hpp>
#include <DDA/QmrCs.hpp>
#include <DDA/Cgnr.hpp>
#include <DDA/GpuCgnr.hpp>
#include <DDA/GpuQmrCs.hpp>
#include <DDA/BicgCs.hpp>
#include <DDA/BicgStab.hpp>
#include <DDA/GpuBicgCs.hpp>
#include <DDA/GpuBicgStab.hpp>
#include <DDA/Geometry.hpp>
#include <DDA/GeometryParser.hpp>
#include <DDA/DipoleGeometry.hpp>
#include <DDA/DMatrixCpu.hpp>
#include <DDA/DMatrixGpu.hpp>
#include <DDA/Beam.hpp>
#include <DDA/FarFieldCalc.hpp>
#include <DDA/PolarizabilityDescription.hpp>
#include <DDA/CpuFieldCalculator.hpp>
#include <DDA/GpuFieldCalculator.hpp>
#include <DDA/DataFilesDDAUtil.hpp>
#include <DDA/AbsCross.hpp>
#include <DDA/Shapes.hpp>
#include <DDA/Load.hpp>
#include <DDA/Options.hpp>
#include <DDA/GpuFFTPlans.hpp>

#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <cmath>

using namespace DDA;

namespace {
  struct DDAOptions {
    Core::OStream out;
    Core::OStream log;
    Core::ProfilingDataPtr prof;
    boost::filesystem::path outputDir;
    boost::program_options::variables_map map;
    std::string cmdLine;

    DDAOptions (const Core::OStream& out, const Core::OStream& log, Core::ProfilingDataPtr prof, const boost::filesystem::path& outputDir, const boost::program_options::variables_map& map, const std::string& cmdLine) : out (out), log (log), prof (prof), outputDir (outputDir), map (map), cmdLine (cmdLine) {}
  };
}

template <class ftype>
static void createGeometryDDAParams (const DDAOptions& opt, boost::shared_ptr<DDAParams<ftype> >& ddaParams, boost::shared_ptr<const Beam<ftype> >& beam, boost::shared_ptr<const CoupleConstants<ftype> >& cc1, boost::shared_ptr<const CoupleConstants<ftype> >& cc2, bool& symmetric, bool supportNonPot, cuint32_t procs = 1) {
  boost::scoped_ptr<Core::ProfileHandle> p1;
  p1.reset (new Core::ProfileHandle (opt.prof, "ddaParams cr"));

  // true = dipole model is symmatric with respect to rotation by 90 degrees over the z-axis
  symmetric = true;
  EMSim::Length lambda = opt.map["lambda"].as<EMSim::Length> ();
  boost::shared_ptr<Geometry> geometry = parseGeometry (opt.map["geometry"].as<std::string> ());

  Math::Vector3<ldouble> propNotNorm = opt.map["prop"].as<Math::Vector3<ldouble> > ();
  beam = Beam<ftype>::parseBeam (static_cast<Math::Vector3<ftype> > (propNotNorm), opt.map["beam"].as<std::string> ());
  if (Math::abs2 (beam->prop () - Math::Vector3<ldouble> (0, 0, 1)) > 1e-12l)
    symmetric = false;
  if (!beam->rotSym90 ())
    symmetric = false;

  Math::Vector3<ldouble> orient = opt.map["orient"].as<Math::Vector3<ldouble> > ();
  EMSim::Rotation<ldouble> orientation = EMSim::Rotation<ldouble>::fromZYZDeg (orient.x (), orient.y (), orient.z ());
  if (orientation.squaredDifference (EMSim::Rotation<ldouble>::none ()) > 1e-12l)
    symmetric = false;

  ldouble gridUnit = -1;
  if (opt.map.count ("grid-unit")) {
    gridUnit = opt.map["grid-unit"].as<EMSim::Length> ();
    ASSERT (gridUnit > 0);
  }

  boost::shared_ptr<DipoleGeometry> dipoleGeometry;
  if (geometry) {
    std::vector<Math::DiagMatrix3<cldouble> > components = opt.map["m"].as<std::vector<Math::DiagMatrix3<cldouble> > > ();
    ASSERT (components.size () == 0);

    if (gridUnit < 0) {
      std::vector<ldouble> dimensions;
      std::vector<Math::DiagMatrix3<cldouble > > materials;
      geometry->getDimensionsMaterials (dimensions, materials);

      // Calculate default value for gridUnit based on material constants and size of the particle, similar to adda
      ldouble tmp2 = 0;
      for (size_t i = 0; i < materials.size (); i++)
        for (int j = 0; j < 3; j++)
          tmp2 = std::max (tmp2, std::norm (materials[i][j]));
      ldouble dpl_def = 10 * std::sqrt (tmp2);
      gridUnit = lambda.valueAs<ldouble> () / dpl_def;
      BOOST_FOREACH (ldouble length, dimensions) {
        ldouble gu = length / 16.0;
        gridUnit = std::min (gridUnit, gu);
      }
    }

    geometry->orientation () = orientation;
    geometry->setPeriodicity (opt.map["periodicity-1"].as<Math::Vector3<EMSim::Length> > (),
                              opt.map["periodicity-2"].as<Math::Vector3<EMSim::Length> > ());
    dipoleGeometry = geometry->createDipoleGeometry (gridUnit);
  } else { // Load file
    std::string s = opt.map["geometry"].as<std::string> ();
    ASSERT (s.substr (0, 5) == "load:");
    s = s.substr (5);
    if (s.substr (0, 5) != "file=")
      ABORT_MSG ("Geometry `load:" + s + "' parameter does not start with `file='");
    s = s.substr (5);
    boost::filesystem::path filename = s;
    if (boost::filesystem::is_directory (filename))
      filename /= "Geometry.hdf5";
    ASSERT_MSG (boost::filesystem::is_regular_file (filename), filename.BOOST_FILE_STRING);
    if (HDF5::File::isHDF5 (filename)) {
      HDF5::File file = HDF5::File::open (filename, H5F_ACC_RDONLY);
      boost::shared_ptr<EMSim::DataFiles::DDADipoleListGeometryFileLight> geometryFile = HDF5::matlabDeserialize<EMSim::DataFiles::DDADipoleListGeometryFileLight> (file);
      ASSERT_MSG (geometryFile->Geometry, "No Geometry object in geometry file");
      const EMSim::DataFiles::DDADipoleListGeometryLight& geometry = *geometryFile->Geometry;

      if (gridUnit < 0) {
        if (geometry.GridSpacing) {
          ASSERT (geometry.GridSpacing->m11 () == geometry.GridSpacing->m22 ());
          ASSERT (geometry.GridSpacing->m11 () == geometry.GridSpacing->m33 ());
          gridUnit = geometry.GridSpacing->m11 ();
          ASSERT (gridUnit > 0);
        } else {
          ABORT_MSG ("No --grid-unit option and no GridSpacing object in Geometry object in geometry file");
        }
      }

      Math::Vector3<ldouble> periodicity1 = opt.map["periodicity-1"].as<Math::Vector3<EMSim::Length> > ();
      Math::Vector3<ldouble> periodicity2 = opt.map["periodicity-2"].as<Math::Vector3<EMSim::Length> > ();
      if (!(periodicity1.x () || periodicity1.y () || periodicity1.z ()
            || periodicity2.x () || periodicity2.y () || periodicity2.z ())) {
        if (geometry.Periodicity) {
          if (geometry.Periodicity->size () == 0) {
            periodicity1 = periodicity2 = Math::Vector3<ldouble> (0, 0, 0);
          } else if (geometry.Periodicity->size () == 1) {
            periodicity1 = (*geometry.Periodicity)[0];
            periodicity2 = Math::Vector3<ldouble> (0, 0, 0);
          } else if (geometry.Periodicity->size () == 2) {
            periodicity1 = (*geometry.Periodicity)[0];
            periodicity2 = (*geometry.Periodicity)[1];
          } else {
            ABORT ();
          }
        }
      }

      dipoleGeometry = boost::make_shared<DipoleGeometry> (gridUnit, periodicity1, periodicity2);

      ASSERT_MSG (geometry.DipolePositions, "No DipolePositions object in Geometry object in geometry file");
      ASSERT_MSG (geometry.DipoleMaterialIndices, "No DipoleMaterialIndices object in Geometry object in geometry file");
      size_t count = geometry.DipolePositions->size ();
      ASSERT (geometry.DipoleMaterialIndices->size () == count);
      for (size_t i = 0; i < count; i++) {
        Math::Vector3<uint32_t> pos = (*geometry.DipolePositions)[i];
        uint8_t material = (*geometry.DipoleMaterialIndices)[i];
        dipoleGeometry->addDipole (pos.x (), pos.y (), pos.z (), material);
      }

      if (geometry.GridOrigin) {
        dipoleGeometry->origin () = *geometry.GridOrigin;
      } else {
        dipoleGeometry->moveToCenter ();
      }

      if (geometry.Orientation && opt.map["orient"].defaulted ()) {
        dipoleGeometry->orientation (EMSim::Rotation<ldouble> (*geometry.Orientation));
      } else {
        dipoleGeometry->orientation (orientation);
      }

      std::vector<Math::DiagMatrix3<cldouble> > components = opt.map["m"].as<std::vector<Math::DiagMatrix3<cldouble> > > ();
      if (components.size () == 0) {
        ASSERT_MSG (geometry.RefractiveIndices, "No -m option given and no RefractiveIndices object in Geometry object in geometry file");
        BOOST_FOREACH (Math::DiagMatrix3<cldouble > value, *geometry.RefractiveIndices)
          components.push_back (value);
        dipoleGeometry->materials () = components;
        ASSERT (dipoleGeometry->materials ().size () >= dipoleGeometry->matCount ());
      } else {
        dipoleGeometry->materials () = components;
      }
    } else {
      if (gridUnit < 0)
        ABORT_MSG ("No --grid-unit option given but needed for loading text geometry file");
      std::vector<Math::DiagMatrix3<cldouble> > components = opt.map["m"].as<std::vector<Math::DiagMatrix3<cldouble> > > ();
      ASSERT (components.size () != 0);
      dipoleGeometry = boost::make_shared<DipoleGeometry> (gridUnit,
                                                           opt.map["periodicity-1"].as<Math::Vector3<EMSim::Length> > (),
                                                           opt.map["periodicity-2"].as<Math::Vector3<EMSim::Length> > ());
      loadDipoleGeometry (*dipoleGeometry, filename);
      dipoleGeometry->orientation (orientation);
      dipoleGeometry->materials () = components;
      ASSERT (dipoleGeometry->materials ().size () >= dipoleGeometry->matCount ());
    }
  }

  const std::vector<Math::DiagMatrix3<cldouble> >& components = dipoleGeometry->materials ();
  for (size_t i = 0; i < components.size (); i++)
    for (size_t j = 0; j < 3; j++)
      if (components[i].m11 () != components[i][j])
        symmetric = false;
  if (!geometry || !geometry->isSymmetric ())
    symmetric = false;
  if (opt.map.count ("no-symmetry"))
    symmetric = false;
  dipoleGeometry->normalize ();

  ddaParams.reset (new DDAParams<ftype> (geometry, opt.map["geometry"].as<std::string> (), dipoleGeometry, lambda.valueAs <ftype> (), supportNonPot, procs, opt.map["fft-grid"].as<Math::Vector3<uint32_t> > (), static_cast<ftype> (opt.map["gamma"].as<ldouble> ())));

  DataFiles::createDDADipoleListGeometryFile (ddaParams->dipoleGeometry ())->write (opt.outputDir / "Geometry", opt.map.count ("write-txt") ? (std::string) ".txt" : boost::optional<std::string> ());

  boost::shared_ptr<const PolarizabilityDescription<ftype> > polDesc = PolarizabilityDescription<ftype>::parsePolDesc (opt.map["pol"].as<std::string> ());

  cc1 = boost::shared_ptr<CoupleConstants<ftype> > (new CoupleConstants<ftype> (*ddaParams, *beam, BEAMPOLARIZATION_1, *polDesc));
  cc2 = boost::shared_ptr<CoupleConstants<ftype> > (new CoupleConstants<ftype> (*ddaParams, *beam, BEAMPOLARIZATION_2, *polDesc));

  if (opt.map.count ("verbose"))
    ddaParams->dump (*beam, *cc1, *cc2, opt.out);
  opt.out << std::flush;
  opt.out << "Particle grid: " << ddaParams->dipoleGeometry ().box ()
      << ", FFT grid: " << ddaParams->gridSize () << std::endl;
  opt.out << "Total dipoles: " << ddaParams->dipoleGeometry ().box ().x () * ddaParams->dipoleGeometry ().box ().y () * ddaParams->dipoleGeometry ().box ().z ()
      << ", Non-void dipoles: " << ddaParams->cnvCount () << std::endl;
  ddaParams->dump (*beam, *cc1, *cc2, opt.log);
  opt.log << std::flush;
  p1.reset ();
}

template <class ftype>
static void outputCrossSections (const boost::filesystem::path& output, const Core::OStream& out, const DDAParams<ftype>& ddaParams, FieldCalculator<ftype>& calculator, const boost::shared_ptr<const Beam<ftype> >& beam, const boost::shared_ptr<const CoupleConstants<ftype> >& cc, uint32_t polarizationNr, const std::string& label, const std::vector<std::complex<ftype> >& result, BeamPolarization pol) {
  ftype normFactor;
  if (ddaParams.periodicityDimension () == 0) {
    ftype a_eq = std::pow (FPConst<ftype>::three_over_four_pi * static_cast<ftype> (ddaParams.nvCount ()), FPConst<ftype>::one_third) * ddaParams.gridUnit ();
    normFactor = 1 / (FPConst<ftype>::pi * a_eq * a_eq);
  } else if (ddaParams.periodicityDimension () == 1) {
    return; // TODO: implement
  } else if (ddaParams.periodicityDimension () == 2) {
    return; // TODO: implement
  } else {
    ABORT ();
  }
  EMSim::CrossSection cs;
  cs.Cext = beam->extCross (ddaParams, calculator, result, pol);
  cs.Cabs = AbsCross<ftype>::absCross (ddaParams, result, *cc);
  cs.setSca ();
  cs.setQFromC (normFactor);
  EMSim::DataFiles::createCrossSectionFile (polarizationNr, cs)->write (output, (std::string) ".txt");
  cs.print (out, label);
}

template <class ftype>
static void createResOutput (const DDAOptions& opt, const DDAParams<ftype>& ddaParams, FieldCalculator<ftype>& calculator, bool symmetric, const boost::shared_ptr<IterativeSolverBase<ftype> >& solver, const boost::shared_ptr<const Beam<ftype> > beam, const boost::shared_ptr<const CoupleConstants<ftype> >& cc1, const boost::shared_ptr<const CoupleConstants<ftype> >& cc2) {
  typedef std::complex<ftype> ctype;
  boost::scoped_ptr<Core::ProfileHandle> p1;

  ftype epsilon = std::pow (10.0f, -static_cast<ftype> (opt.map["epsilon"].as<ldouble> ()));

  if (opt.map.count ("profiling-run")) {
    solver->setCoupleConstants (cc1);
    solver->profilingRun (*opt.out, *opt.log, opt.prof);
    return;
  }

  std::vector<std::string> farFieldOptions;

  if (opt.map.count ("far-field"))
    farFieldOptions = opt.map["far-field"].as<std::vector<std::string> > ();
  if (opt.map.count ("efield") || opt.map.count ("mueller-matrix")) {
    uint32_t ntheta;
    if (opt.map.count ("ntheta")) {
      ntheta = opt.map["ntheta"].as<uint32_t> ();
    } else {
      cuint32_t nvCount = ddaParams.dipoleGeometry ().nvCount ();
      if (nvCount < 1000)
        ntheta = 180;
      else if (nvCount < 10000)
        ntheta = 360;
      else if (nvCount < 100000)
        ntheta = 720;
      else
        ntheta = 1440;
    }
    std::stringstream str;
    FPRINTF (Core::OStream::get (str), "Plane=0..%.20f/%.20f,90,no-phi", 360.0 - 360.0 / ntheta, ntheta);
    if (opt.map.count ("efield"))
      farFieldOptions.push_back (str.str ());
    str << ",mueller";
    if (opt.map.count ("mueller-matrix"))
      farFieldOptions.push_back (str.str ());
  }
  if (opt.map.count ("efield-grid"))
    farFieldOptions.push_back ("Grid=0..180/91,0..360/61");
  if (opt.map.count ("mueller-matrix-grid"))
    farFieldOptions.push_back ("Grid=0..180/91,0..360/61,mueller");

  std::map<EMSim::GridAngleList, std::vector<FarFieldOption> > farFields;
  typedef std::pair<EMSim::GridAngleList, std::vector<FarFieldOption> > pairType;
  BOOST_FOREACH (const std::string& str, farFieldOptions) {
    FarFieldOption ff = FarFieldOption::parse (str);
    std::map<EMSim::GridAngleList, std::vector<FarFieldOption> >::iterator iter = farFields.find (ff.angleList ());
    if (iter == farFields.end ())
      iter = farFields.insert (std::make_pair (ff.angleList (), std::vector<FarFieldOption> ())).first;
    iter->second.push_back (ff);
  }

  boost::shared_ptr<EMSim::DataFiles::Parameters<EMSim::DataFiles::DDAParameters> > parameters = DataFiles::createParametersDDA (ddaParams);
  parameters->MethodParameters.PolarizabilityType = opt.map["pol"].as<std::string> ();
  parameters->CmdLine = opt.cmdLine;
  parameters->PropagationVector = beam->prop ();
  parameters->BeamString = opt.map["beam"].as<std::string> ();
  parameters->Polarizations.resize (2);
  parameters->Polarizations[0] = beam->getIncPol (BEAMPOLARIZATION_1);
  parameters->Polarizations[1] = beam->getIncPol (BEAMPOLARIZATION_2);

  boost::shared_ptr<EMSim::DataFiles::MieGeometry> mieGeometry;
  boost::shared_ptr<EMSim::DataFiles::Parameters<EMSim::DataFiles::MieParameters> > mieParameters;

  std::vector<ctype> res1, res2;
  if (opt.map.count ("load-dip-pol")) {
    boost::filesystem::path dpDir = opt.map["load-dip-pol"].as<std::string> ();
    Load<ftype>::loadDipPol (dpDir / "DipPol-Pol1", ddaParams, res1);
    if (!symmetric)
      Load<ftype>::loadDipPol (dpDir / "DipPol-Pol2", ddaParams, res2);
  } else {
    opt.out << "Solve " << (symmetric ? "Pol1/Pol2:" : "Pol1:") << std::endl;
    p1.reset (new Core::ProfileHandle (opt.prof, symmetric ? "res12" : "res1"));
    std::vector<ctype> einc (ddaParams.vecSize ());
    beam->createEInc (ddaParams, BEAMPOLARIZATION_1, einc);
    if (opt.map.count ("store-incbeam"))
      DataFiles::createDDAFieldFile<ftype> (ddaParams, parameters, "IncidentBeam", 1, einc)->write (opt.outputDir / "IncBeam-Pol1", opt.map.count ("write-txt") ? (std::string) ".txt" : boost::optional<std::string> ());
    solver->setCoupleConstants (cc1);
    std::vector<ctype> start (0);
    if (opt.map.count ("load-start-dip-pol")) {
      boost::filesystem::path dpDir = opt.map["load-start-dip-pol"].as<std::string> ();
      Load<ftype>::loadDipPol (dpDir / "DipPol-Pol1", ddaParams, start);
    }
    swap (res1, *solver->getPolVec (einc, epsilon, *opt.log, start, opt.prof));
    p1.reset ();
    opt.out << std::endl;
    if (!symmetric) {
      opt.out << "Solve Pol2:" << std::endl;
      p1.reset (new Core::ProfileHandle (opt.prof, "res2"));
      beam->createEInc (ddaParams, BEAMPOLARIZATION_2, einc);
      if (opt.map.count ("store-incbeam"))
        DataFiles::createDDAFieldFile<ftype> (ddaParams, parameters, "IncidentBeam", 2, einc)->write (opt.outputDir / "IncBeam-Pol2", opt.map.count ("write-txt") ? (std::string) ".txt" : boost::optional<std::string> ());
      solver->setCoupleConstants (cc2);
      std::vector<ctype> start (0);
      if (opt.map.count ("load-start-dip-pol")) {
        boost::filesystem::path dpDir = opt.map["load-start-dip-pol"].as<std::string> ();
        Load<ftype>::loadDipPol (dpDir / "DipPol-Pol2", ddaParams, start);
      }
      swap (res2, *solver->getPolVec (einc, epsilon, *opt.log, start, opt.prof));
      p1.reset ();
      opt.out << std::endl;
    }
  }

  if (opt.map.count ("store-dippol") || 1) {
    p1.reset (new Core::ProfileHandle (opt.prof, "output dippol"));
    DataFiles::createDDAFieldFile<ftype> (ddaParams, parameters, "DipolePolarization", 1, res1)->write (opt.outputDir / "DipPol-Pol1", opt.map.count ("write-txt") ? (std::string) ".txt" : boost::optional<std::string> ());
    if (!symmetric)
      DataFiles::createDDAFieldFile<ftype> (ddaParams, parameters, "DipolePolarization", 2, res2)->write (opt.outputDir / "DipPol-Pol2", opt.map.count ("write-txt") ? (std::string) ".txt" : boost::optional<std::string> ());
    p1.reset ();
  }

  if (opt.map.count ("store-intfield")) {
    p1.reset (new Core::ProfileHandle (opt.prof, "output intfield"));
    DataFiles::createDDAFieldFile<ftype> (ddaParams, parameters, "InternalField", 1, *ddaParams.multMat (cc1->chi_inv (), res1))->write (opt.outputDir / "IntField-Pol1", opt.map.count ("write-txt") ? (std::string) ".txt" : boost::optional<std::string> ());
    if (!symmetric)
      DataFiles::createDDAFieldFile<ftype> (ddaParams, parameters, "InternalField", 2, *ddaParams.multMat (cc2->chi_inv (), res2))->write (opt.outputDir / "IntField-Pol2", opt.map.count ("write-txt") ? (std::string) ".txt" : boost::optional<std::string> ());
    p1.reset ();
  }
    
  p1.reset (new Core::ProfileHandle (opt.prof, "output"));
  opt.out << std::endl;
  if (ddaParams.template geometryIs<Shapes::Sphere> () && ddaParams.periodicityDimension () == 0 && dynamic_cast<const Beams::PlaneWave<ftype>*> (beam.get ())) {
    Math::DiagMatrix3<cldouble > m = ddaParams.template geometryAs<Shapes::Sphere> ().material ();
    if (m.m11 () == m.m22 () && m.m11 () == m.m33 ()) {
      mieGeometry = boost::make_shared<EMSim::DataFiles::MieGeometry> ();
      mieGeometry->Type = "Mie";
      mieGeometry->Radius = static_cast<__decltype (mieGeometry->Radius)> (ddaParams.template geometryAs<Shapes::Sphere> ().radius ().template valueAs <ftype> ());
      mieGeometry->RefractiveIndex = (Math::DiagMatrix3<cdouble>) m;
      mieParameters = EMSim::DataFiles::createParametersMie (ddaParams.frequency (), ddaParams.geometryString ());
      mieParameters->CmdLine = opt.cmdLine;
      mieParameters->PropagationVector = beam->prop ();
      mieParameters->BeamString = opt.map["beam"].as<std::string> ();
      mieParameters->Polarizations.resize (2);
      mieParameters->Polarizations[0] = beam->getIncPol (BEAMPOLARIZATION_1);
      mieParameters->Polarizations[1] = beam->getIncPol (BEAMPOLARIZATION_2);
      
      ldouble x = 2 * FPConst<ldouble>::pi * mieGeometry->Radius / ddaParams.lambda ();
      cldouble refrel = mieGeometry->RefractiveIndex.m11 ();
      ldouble qback = 0.0f/0.0f;
      ldouble gsca = 0.0f/0.0f;
      EMSim::CrossSection cs;
      boost::shared_ptr<EMSim::DataFiles::JonesFarField<ldouble> > farField;
      EMSim::BHMie (x, refrel, EMSim::NullAngleList (), 0, farField, cs, qback, gsca);
      //EPRINTVALS (qback, gsca);
      cs.setCFromQ (FPConst<ldouble>::pi * Math::squared (mieGeometry->Radius));
      opt.out << "Mie solution:" << std::endl;
      EMSim::DataFiles::createCrossSectionFile (0, cs)->write (opt.outputDir / "MieCrossSec", (std::string) ".txt");
      cs.print (opt.out, "    ");
      opt.out << std::endl;
    }
  }
  if (symmetric) {
    outputCrossSections (opt.outputDir / "CrossSec-Pol1", opt.out, ddaParams, calculator, beam, cc1, 1, "    ", res1, BEAMPOLARIZATION_1);
    opt.out << std::endl;
    outputCrossSections (opt.outputDir / "CrossSec-Pol2", Core::OStream::openNull (), ddaParams, calculator, beam, cc1, 2, "    ", res1, BEAMPOLARIZATION_1);
  } else {
    outputCrossSections (opt.outputDir / "CrossSec-Pol1", opt.out, ddaParams, calculator, beam, cc1, 1, "Pol1", res1, BEAMPOLARIZATION_1);
    opt.out << std::endl;
    outputCrossSections (opt.outputDir / "CrossSec-Pol2", opt.out, ddaParams, calculator, beam, cc2, 2, "Pol2", res2, BEAMPOLARIZATION_2);
    opt.out << std::endl;
  }
  p1.reset ();

  p1.reset (new Core::ProfileHandle (opt.prof, "farfield"));
  BOOST_FOREACH (const pairType& pair, farFields) {
    FarFieldCalc<ftype>::calcAndStore (opt.outputDir / "Far", ddaParams, calculator, parameters, res1, res2, symmetric, *beam, pair.first, pair.second, opt.map.count ("write-txt"));

    // Mie far field
    if (mieGeometry) {
      ldouble x = 2 * FPConst<ldouble>::pi * mieGeometry->Radius / ddaParams.lambda ();
      cldouble refrel = mieGeometry->RefractiveIndex.m11 ();
      ldouble qback;
      ldouble gsca;
      EMSim::CrossSection cs;
      boost::shared_ptr<EMSim::DataFiles::JonesFarField<ldouble> > farField;
      EMSim::BHMie (x, refrel, pair.first, -1/ddaParams.waveNum (), farField, cs, qback, gsca);
      farField->Frequency[0] = static_cast<double> (ddaParams.frequency ());
      boost::shared_ptr<EMSim::DataFiles::MieGeometry> geometry = boost::make_shared<EMSim::DataFiles::MieGeometry> ();
      BOOST_FOREACH (const FarFieldOption& option, pair.second)
        FarFieldCalc<ldouble>::store (opt.outputDir / "MieFar", mieGeometry, mieParameters, farField, option, opt.map.count ("write-txt"));
    }
  }
  p1.reset ();
}

template <class ftype>
static void ddaCpu (const DDAOptions& opt) {
  typedef std::complex<ftype> ctype;

  boost::scoped_ptr<Core::ProfileHandle> p1;

  const LinAlg::FFTPlanFactory<ftype>& planFactory = LinAlg::getFFTWPlanFactory<ftype> ();

  bool symmetric;
  boost::shared_ptr<DDAParams<ftype> > ddaParamsPtr;
  boost::shared_ptr<const Beam<ftype> > beam;
  boost::shared_ptr<const CoupleConstants<ftype> > cc1, cc2;
  createGeometryDDAParams<ftype> (opt, ddaParamsPtr, beam, cc1, cc2, symmetric, planFactory.supportNonPOTSizes ());
  const DDAParams<ftype>& g = *ddaParamsPtr;

  boost::shared_ptr<IterativeSolverBase<ftype> > solver;
  boost::shared_ptr<MatVecCpu<ftype> > matVec;
  boost::shared_ptr<boost::multi_array<Math::SymMatrix3<ctype>, 3> > dMatrix;
  if (!opt.map.count ("load-dip-pol")) {
    p1.reset (new Core::ProfileHandle (opt.prof, "Dmatrix"));
    opt.out << "Size of DMatrix: " << (g.cgridY () * g.cgridZ () * g.cgridX () * 6 * sizeof (ctype) / 1024 / 1024) << "MB" << std::endl;
    dMatrix.reset (new boost::multi_array<Math::SymMatrix3<ctype>, 3> (boost::extents[g.gridY ()][g.gridZ ()][g.gridX ()], boost::fortran_storage_order ()));
    if (!opt.map.count ("profiling-run"))
      DMatrixCpu<ftype>::createDMatrix (g, planFactory, *dMatrix, beam);
    p1.reset ();

    p1.reset (new Core::ProfileHandle (opt.prof, "cr matvec"));
    matVec.reset (new MatVecCpu<ftype> (g, *dMatrix, planFactory));
    p1.reset ();

    csize_t maxIter = 0;
    if (opt.map.count ("maxiter"))
      maxIter = opt.map["maxiter"].as<size_t> ();
    else
      maxIter = g.dipoleGeometry ().box ().x () * g.dipoleGeometry ().box ().y () * g.dipoleGeometry ().box ().z () * 3;

    p1.reset (new Core::ProfileHandle (opt.prof, "cr solver"));
    if (opt.map["iter"].as<std::string> () == "qmr") {
      solver.reset (new QmrCs<ftype> (g, *matVec, maxIter));
    } else if (opt.map["iter"].as<std::string> () == "cgnr") {
      solver.reset (new Cgnr<ftype> (g, *matVec, maxIter));
    } else if (opt.map["iter"].as<std::string> () == "bicg") {
      solver.reset (new BicgCs<ftype> (g, *matVec, maxIter));
    } else if (opt.map["iter"].as<std::string> () == "bicgstab") {
      solver.reset (new BicgStab<ftype> (g, *matVec, maxIter));
    } else
      ABORT_MSG ("Unknown iterative solver `" + opt.map["iter"].as<std::string> () + "'");
    p1.reset ();
  }

  CpuFieldCalculator<ftype> calculator (g);

  createResOutput (opt, g, calculator, symmetric, solver, beam, cc1, cc2);
}

template <class ftype>
static void ddaCl (const DDAOptions& opt) {
  typedef std::complex<ftype> ctype;

  boost::scoped_ptr<Core::ProfileHandle> p1;

  cl::Context context = OpenCL::createContext (opt.map["device"].as<std::string> (), opt.out);
#ifdef DDAMAIN_ADDITIONAL_CONTEXT_SETUP
  DDAMAIN_ADDITIONAL_CONTEXT_SETUP
#endif
  OpenCL::StubPool pool (context);
  if (opt.map.count ("sync"))
    pool.options ().enableSync (true);

  const LinAlg::GpuFFTPlanFactory<ftype>& planFactory = getPlanFactory<ftype> (opt.map, pool);

  bool symmetric;
  boost::shared_ptr<DDAParams<ftype> > ddaParamsPtr;
  boost::shared_ptr<const Beam<ftype> > beam;
  boost::shared_ptr<const CoupleConstants<ftype> > cc1, cc2;
  createGeometryDDAParams<ftype> (opt, ddaParamsPtr, beam, cc1, cc2, symmetric, planFactory.supportNonPOTSizes (), context.getInfo<CL_CONTEXT_DEVICES> ().size ());
  const DDAParams<ftype>& g = *ddaParamsPtr;

  std::vector<cl::CommandQueue> queues (g.procs () ());
  for (size_t i = 0; i < g.procs (); i++)
    queues[i] = cl::CommandQueue (context, context.getInfo<CL_CONTEXT_DEVICES>()[i], 0);

  Core::OStream memStream = Core::OStream::open (opt.outputDir / "meminfo");
  if (opt.map.count ("mem-info"))
    memStream = Core::OStream::tee (Core::OStream::getStderr (), memStream);
  struct MyVectorAccounting : public OpenCL::VectorAccounting {
    Core::OStream out;
    csize_t sum;
    MyVectorAccounting (const Core::OStream& out) : out (out), sum (0) {}
    std::auto_ptr<Handle> allocate (const std::string& name, const std::type_info& typeInfo, csize_t count, csize_t elementSize) {
      std::stringstream str;
      //sum += count * elementSize;
      sum += (count * elementSize + 1023) / 1024 * 1024;
      str << "'" << name << "': OpenCL::Vector<" << Core::Type::getName (typeInfo) << "> (" << count << ") = " << (count * elementSize + 1023) / 1024 << " kB";
      out << "Alloc " << str.str () << ", sum = " << (sum + 1023) / 1024 << " kB" << std::endl;
      struct MyHandle : Handle {
        Core::OStream out;
        std::string s;
        MyHandle (const Core::OStream& out, const std::string& s) : out (out), s (s) {}
        ~MyHandle () {
          //out << "Free " << s << std::endl;
        }
      };
      return std::auto_ptr<Handle> (new MyHandle (out, str.str ()));
    }
  } accountingI (memStream);
  OpenCL::VectorAccounting& accounting = accountingI;

  boost::shared_ptr<IterativeSolverBase<ftype> > solver;
  boost::shared_ptr<GpuMatVec<ftype> > matVecInst;
  boost::scoped_ptr<OpenCL::MultiGpuVector<ctype> > dMatrixInst;
  boost::scoped_ptr<boost::multi_array<Math::SymMatrix3<ctype>, 3> > dMatrixCpuInst;
  // boost::multi_array<T, n> does not inherit from boost::const_multi_array_ref<T, n> (which is equivalent to boost::const_multi_array_ref<T, n, const T*>) but from boost::const_multi_array_ref<T, n, T*>, so converting dMatrixCpuInst to const_multi_array_ref<> will create a new object which must be stored somewhere until createResOutput() is finished.
  boost::scoped_ptr<boost::const_multi_array_ref<Math::SymMatrix3<ctype>, 3> > dMatrixCpuRef;
  if (!opt.map.count ("load-dip-pol")) {
    if (opt.map.count ("dmatrix-host")) {
      p1.reset (new Core::ProfileHandle (opt.prof, "Dmatrix"));
      opt.out << "Size of DMatrix: " << (g.cgridY () * g.cgridZ () * g.cgridX () * 6 * sizeof (ctype) / 1024 / 1024) << "MB" << std::endl;
      dMatrixCpuInst.reset (new boost::multi_array<Math::SymMatrix3<ctype>, 3> (boost::extents[g.gridY ()][g.gridZ ()][g.gridX ()], boost::fortran_storage_order ()));
      dMatrixCpuRef.reset (new boost::const_multi_array_ref<Math::SymMatrix3<ctype>, 3> (*dMatrixCpuInst));
      const LinAlg::FFTPlanFactory<ftype>& cpuPlanFactory = LinAlg::getFFTWPlanFactory<ftype> ();
      if (!opt.map.count ("profiling-run"))
        DMatrixCpu<ftype>::createDMatrix (g, cpuPlanFactory, *dMatrixCpuInst, beam);
      p1.reset ();

      p1.reset (new Core::ProfileHandle (opt.prof, "cr matvec"));
      matVecInst = GpuMatVec<ftype>::create (queues, g, *dMatrixCpuRef, planFactory, pool, accounting, opt.prof);
      p1.reset ();
    } else {
      p1.reset (new Core::ProfileHandle (opt.prof, "Dmatrix"));
      opt.out << "Size of DMatrix: " << (g.cgridY () * g.cgridZ () * g.cgridX () * 6 * sizeof (ctype) / 1024 / 1024) << "MB" << std::endl;
      std::vector<size_t> dMatrixSizes (g.procs () ());
      for (size_t i = 0; i < g.procs (); i++)
        dMatrixSizes[i] = (g.cgridY () * g.cgridZ () * g.localCGridX (i) * 6) ();
      dMatrixInst.reset (new OpenCL::MultiGpuVector<ctype> (pool, queues, dMatrixSizes, accounting, "dMatrix"));
      if (!opt.map.count ("profiling-run"))
        DMatrixGpu<ftype>::createDMatrix (pool, queues, g, planFactory, *dMatrixInst, beam);
      p1.reset ();

      p1.reset (new Core::ProfileHandle (opt.prof, "cr matvec"));
      matVecInst = GpuMatVec<ftype>::create (queues, g, *dMatrixInst, planFactory, pool, accounting, opt.prof);
      p1.reset ();
    }
    GpuMatVec<ftype>& matVec = *matVecInst;

    csize_t maxIter = 0;
    if (opt.map.count ("maxiter"))
      maxIter = opt.map["maxiter"].as<size_t> ();
    else
      maxIter = g.dipoleGeometry ().box ().x () * g.dipoleGeometry ().box ().y () * g.dipoleGeometry ().box ().z () * 3;

    p1.reset (new Core::ProfileHandle (opt.prof, "cr solver"));
    if (opt.map["iter"].as<std::string> () == "qmr") {
      solver.reset (new GpuQmrCs<ftype> (pool, queues, g, matVec, maxIter, accounting));
    } else if (opt.map["iter"].as<std::string> () == "cgnr") {
      solver.reset (new GpuCgnr<ftype> (pool, queues, g, matVec, maxIter, accounting));
    } else if (opt.map["iter"].as<std::string> () == "bicg") {
      solver.reset (new GpuBicgCs<ftype> (pool, queues, g, matVec, maxIter, accounting));
    } else if (opt.map["iter"].as<std::string> () == "bicgstab") {
      solver.reset (new GpuBicgStab<ftype> (pool, queues, g, matVec, maxIter, accounting, opt.prof));
    } else
      ABORT_MSG ("Unknown iterative solver `" + opt.map["iter"].as<std::string> () + "'");
    p1.reset ();
  }

  GpuFieldCalculator<ftype> calculator (pool, accounting, g, opt.prof);

  createResOutput (opt, g, calculator, symmetric, solver, beam, cc1, cc2);
}

int ddaMain (int argc, char** argv) {
  Core::OStream excout = Core::OStream::getStdout ();
  try {
#ifdef DDAMAIN_ADDITIONAL_STARTUP
    DDAMAIN_ADDITIONAL_STARTUP
#endif

    Options options;

    const boost::optional<Options::Map> mapOpt = options.parse (argc, argv);
    if (!mapOpt)
      return 1;
    const Options::Map& map = *mapOpt;

    if (map.count ("help")) {
      options.help ();
      return 0;
    }

    boost::filesystem::path outputDir;
    boost::shared_ptr<EMSim::OutputDirectory> outputDirectory;
    if (map.count ("output-dir")) {
      if (map.count ("tag")) {
        Core::OStream::getStderr ().fprintf ("Error: Got both --output-dir and --tag\n");
        return 1;
      }

      outputDir = map["output-dir"].as<std::string> ();

      if (!boost::filesystem::exists (outputDir))
        boost::filesystem::create_directory (outputDir);
    } else {
      boost::filesystem::path baseDir;
      if (map.count ("output-parent-dir"))
        baseDir = map["output-parent-dir"].as<std::vector<std::string> > ().back ();
      else
        baseDir = Core::getExecutingPath () / "output";
      outputDirectory = boost::make_shared<EMSim::OutputDirectory> (baseDir);

      outputDirectory->createTag ("running");

      outputDir = outputDirectory->path ();
    }

    ASSERT (!map.count ("cpu") || !map.count ("opencl"));

    Core::OStream log = Core::OStream::open (outputDir / "log");
    Core::OStream out = Core::OStream::tee (Core::OStream::getStdout (), log);
    excout = out;

    boost::filesystem::path absoluteOutputDir = boost::filesystem::system_complete (outputDir).normalize ();
    out << "Output directory: " << absoluteOutputDir << std::endl;

    Core::ProfilingData prof (true);
    DDAOptions opt (out, log, prof, outputDir, map, options.getParameterString (map));

    out << "Command: " << opt.cmdLine << std::endl;

    std::string ftypestr = map["ftype"].as<std::string> ();
    if (ftypestr == "float") {
      if (map.count ("cpu"))
        ddaCpu<float> (opt);
      else
        ddaCl<float> (opt);
    } else if (ftypestr == "double") {
      if (map.count ("cpu"))
        ddaCpu<double> (opt);
      else
        ddaCl<double> (opt);
    } else if (ftypestr == "ldouble") {
      if (map.count ("cpu"))
        ddaCpu<ldouble> (opt);
      else
        ABORT_MSG ("OpenCL does not support long double precision");
    } else {
      out.fprintf ("ftype has invalid value `%s'\n", ftypestr);
      return 1;
    }

    Core::OStream::open (outputDir / "prof") << prof.toString () << std::endl;

    if (outputDirectory) {
      if (map.count ("tag")) {
        BOOST_FOREACH (const std::string& tag, map["tag"].as<std::vector<std::string> > ())
          outputDirectory->createTag (tag);
      }
      outputDirectory->createTag ("latest");
    }

    out << "Overall time " << prof.getOverallTimes ().toString () << std::endl;
    out << "Output directory: " << absoluteOutputDir << std::endl;
  } catch (Core::HelpResultException& e) {
    excout << e.info ();
    return 0;
  } catch (Core::Exception& e) {
    excout << "Error: ";
    excout << Core::Type::getName (typeid (e)) << ": " << std::flush;
    e.writeTo (*excout);
    excout << std::endl;
    return 1;
  } catch (std::exception& e) {
    excout << "Error: ";
    excout << Core::Type::getName (typeid (e)) << ": ";
    excout << e.what () << std::endl;
    excout << Core::StackTrace (Core::StackTrace::createFromCurrentThread).toString () << std::endl;
    return 1;
  }

  return 0;
}
