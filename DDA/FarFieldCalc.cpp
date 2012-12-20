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

#include "FarFieldCalc.hpp"

#include <Core/ProgressBar.hpp>
#include <Core/StringUtil.hpp>
#include <Core/HelpResultException.hpp>

#include <EMSim/FarField.hpp>

#include <DDA/FieldCalculator.hpp>
#include <DDA/DDAParams.hpp>
#include <DDA/Beam.hpp>
#include <DDA/DataFilesDDAUtil.hpp>

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

namespace DDA {
  FarFieldOption::FarFieldOption (std::string outputName, EMSim::GridAngleList angleList, bool storeJones, bool storeMueller, bool noPhi) :
    outputName_ (outputName),
    angleList_ (angleList),
    storeJones_ (storeJones),
    storeMueller_ (storeMueller),
    noPhi_ (noPhi)
  {
  }
  FarFieldOption::~FarFieldOption () {
  }

  FarFieldOption FarFieldOption::parse (const std::string& str) {
    if (str == "help") {
      std::stringstream str;
      str
        << "Values for --far-field option:" << std::endl
        << "FileName=theta,phi[,jones/mueller/both][,no-phi]" << std::endl
        << "theta and phi can be either a single number (an angle in deg) or" << std::endl
        << "of the form startAngle..endAngle/angleCount (both angles in deg)" << std::endl
        << "Appending 'jones' will output only the jones matrix, 'mueller' will output" << std::endl
        << "only the mueller matrix and 'both' wil output both." << std::endl
        << "Default is to output only the jones matrix." << std::endl
        << "The option no-phi causes only theta values to be printed into text output files." << std::endl
        << "The output will be written for Far<FileName>.hdf5/.txt/.mueller.txt" << std::endl;
      throw Core::HelpResultException (str.str ());
    }

    std::size_t equal = str.rfind ('=');
    ASSERT (equal != std::string::npos);
    std::string filename = str.substr (0, equal);
    BOOST_FOREACH (char c, filename) {
      ASSERT (c);
      ASSERT (c != '/');
    }

    EMSim::StringParser parser (str.substr (equal + 1));
    EMSim::GridAngleList grid;
    EMSim::parse (parser, grid);

    // store only jones matrix by default
    bool storeJones = true;
    bool storeMueller = false;
    bool noPhi = false;
    if (!parser.eof ()) {
      ASSERT (parser.read () == ',');
      bool haveOpt = false;
      BOOST_FOREACH (const std::string& option, Core::split (parser.remaining (), ",")) {
        if (option == "jones") {
          ASSERT (!haveOpt);
          haveOpt = true;
          storeJones = true;
          storeMueller = false;
        } else if (option == "mueller") {
          ASSERT (!haveOpt);
          haveOpt = true;
          storeJones = false;
          storeMueller = true;
        } else if (option == "both") {
          ASSERT (!haveOpt);
          haveOpt = true;
          storeJones = true;
          storeMueller = true;
        } else if (option == "none") { // probably rather useless except for testing
          ASSERT (!haveOpt);
          haveOpt = true;
          storeJones = false;
          storeMueller = false;
        } else if (option == "no-phi") {
          ASSERT (!noPhi);
          noPhi = true;
        } else {
          ABORT_MSG ("Invalid option '" + option + "' for far field, see '--far-field help' for more information");
        }
      }
    }

    return FarFieldOption (filename, grid, storeJones, storeMueller, noPhi);
  }

  template <class ftype>
  boost::shared_ptr<std::vector<EMSim::FarFieldEntry<ftype> > > FarFieldCalc<ftype>::calcEField (const DDAParams<ftype>& ddaParams, FieldCalculator<ftype>& calculator, const EMSim::AngleList& angles, const std::vector<std::complex<ftype> >& pvec, Math::Vector3<ldouble> prop2, Math::Vector3<ftype> incPolX, Math::Vector3<ftype> incPolY) {
    typedef std::complex<ftype> ctype;

    Math::Vector3<ftype> prop = (Math::Vector3<ftype>) prop2;

    calculator.setPVec (pvec);

    size_t count = angles.count ();
    boost::shared_ptr<std::vector<EMSim::FarFieldEntry<ftype> > > result = boost::make_shared<std::vector<EMSim::FarFieldEntry<ftype> > > (count);

    Core::ProgressBar progress (Core::OStream::getStderr (), count);
    for (size_t i = 0; i < count; i++) {
      if (i % 1000 == 0)
        progress.update (i, Core::sprintf ("EField %s / %s", i, count));
      std::pair<ldouble, ldouble> thetaPhi = angles.getThetaPhi (i);
      ftype sinTheta = std::sin (static_cast<ftype> (thetaPhi.first));
      ftype cosTheta = std::cos (static_cast<ftype> (thetaPhi.first));
      ftype sinPhi = std::sin (static_cast<ftype> (thetaPhi.second));
      ftype cosPhi = std::cos (static_cast<ftype> (thetaPhi.second));
      Math::Vector3<ftype> scatteringDirection = cosTheta * prop + sinTheta * (cosPhi * incPolX + sinPhi * incPolY);
      Math::Vector3<ctype> ebuff = calculator.calcField (scatteringDirection); // scattered electric field
      if (ddaParams.periodicityDimension () == 0) {
      } else if (ddaParams.periodicityDimension () == 1) {
        ABORT_MSG ("Far field output for periodic structures not implemented"); // TODO: implement
      } else if (ddaParams.periodicityDimension () == 2) {
        ABORT_MSG ("Far field output for periodic structures not implemented"); // TODO: implement
      } else {
        ABORT ();
      }
      // split into perpendicular and parallel components
      Math::Vector3<ftype> incPolPerpendicular = sinPhi * incPolX - cosPhi * incPolY;
      Math::Vector3<ftype> incPolParallel = -sinTheta * prop + cosTheta * (cosPhi * incPolX + sinPhi * incPolY);
      //Core::OStream::getStdout () << prop << " " << incPolPerpendicular << " " << incPolParallel << " " << scatteringDirection << std::endl;
      (*result)[i].perpendicular () = ebuff * Math::Vector3<ctype> (incPolPerpendicular); // ebuff projected onto perpendicular polarization vector
      (*result)[i].parallel () = ebuff * Math::Vector3<ctype> (incPolParallel); // ebuff projected onto parallel polarization vector;
    }
    progress.finish (Core::sprintf ("EField %s / %s", count, count));
    progress.cleanup ();

    return result;
  }


  template <class ftype>
  void FarFieldCalc<ftype>::calcAndStore (const boost::filesystem::path& outputPrefix, const DDAParams<ftype>& ddaParams, FieldCalculator<ftype>& calculator, const boost::shared_ptr<EMSim::DataFiles::Parameters<EMSim::DataFiles::DDAParameters> >& parameters, const std::vector<std::complex<ftype> >& res1, const std::vector<std::complex<ftype> >& res2, bool symmetric, const Beam<ftype>& beam, const EMSim::AngleList& angleList, const std::vector<FarFieldOption>& options, bool writeTxt) {
    typedef std::complex<ftype> ctype;
    boost::shared_ptr<std::vector<EMSim::FarFieldEntry<ftype> > > eField1 = FarFieldCalc<ftype>::calcEField (ddaParams, calculator, angleList, res1, ddaParams.dipoleGeometry ().orientationInverse () * beam.prop (), beam.getIncPolP (ddaParams.dipoleGeometry (), BEAMPOLARIZATION_2), beam.getIncPolP (ddaParams.dipoleGeometry (), BEAMPOLARIZATION_1));
    boost::shared_ptr<std::vector<EMSim::FarFieldEntry<ftype> > > eField2;
    if (symmetric)
      eField2 = calcEField (ddaParams, calculator, angleList, res1, ddaParams.dipoleGeometry ().orientationInverse () * beam.prop (), beam.getIncPolP (ddaParams.dipoleGeometry (), BEAMPOLARIZATION_1), -beam.getIncPolP (ddaParams.dipoleGeometry (), BEAMPOLARIZATION_2));
    else
      eField2 = calcEField (ddaParams, calculator, angleList, res2, ddaParams.dipoleGeometry ().orientationInverse () * beam.prop (), beam.getIncPolP (ddaParams.dipoleGeometry (), BEAMPOLARIZATION_2), beam.getIncPolP (ddaParams.dipoleGeometry (), BEAMPOLARIZATION_1));
    boost::shared_ptr<EMSim::DataFiles::JonesFarField<ftype> > farField = EMSim::JonesCalculus<ftype>::computeJonesFarField (angleList, *eField1, *eField2, ddaParams.frequency ());
    BOOST_FOREACH (const FarFieldOption& option, options)
      store (outputPrefix, DataFiles::createDDADipoleListGeometry (ddaParams.dipoleGeometry (), false), parameters, farField, option, writeTxt);
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, FarFieldCalc)
}
