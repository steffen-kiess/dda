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

#include "Options.hpp"

#include <Core/StringUtil.hpp>
#include <Core/OStream.hpp>

#include <Math/DiagMatrix3.hpp>
#include <Math/Float.hpp>

#include <EMSim/Length.hpp>
#include <EMSim/Parse.hpp>

#include <DDA/FPConst.hpp>
#include <DDA/AddaOptions.hpp>
#include <DDA/ToString.hpp>

#include <boost/foreach.hpp>

namespace DDA {
  Options::Options () {
    positional_.add ("positional-argument", -1);

    description_.add_options ()
      ("help", "Show help")
      ("verbose", "Be verbose")

      ("lambda", boost::program_options::value<EMSim::Length> ()->default_value (EMSim::Length::fromMicroM (FPConst<ldouble>::two_pi)), "Wavelength of the incident light")
      ("prop", boost::program_options::value<Math::Vector3<ldouble> > ()->default_value (Math::Vector3<ldouble> (0, 0, 1)), "Direction of the incident beam")
      ("beam", boost::program_options::value<std::string> ()->default_value ("plane"), "Incident beam shape, see '--beam help' for more information")
      ("pol", boost::program_options::value<std::string> ()->default_value ("ldr"), "Polarizability description, see '--pol help' for more information")
      ("orient", boost::program_options::value<Math::Vector3<ldouble> > ()->default_value (Math::Vector3<ldouble> (0, 0, 0)), "Particle orientation (zyz-notation)")

      ("geometry", boost::program_options::value<std::string> ()->default_value ("sphere:3.35103um,1.5"), "Geometry, see '--geometry help' for more information")
      ("grid-unit", boost::program_options::value<EMSim::Length> (), "Distance between two dipoles")
      ("m,m", boost::program_options::value<std::vector<std::string> > ()->default_value (std::vector<std::string> (), "1.5"), "Refractive indices")
      ("no-symmetry", "Ignore symmetry in particle")

      ("periodicity-1", boost::program_options::value<Math::Vector3<EMSim::Length> > ()->default_value (Math::Vector3<EMSim::Length> (EMSim::Length::fromM (0), EMSim::Length::fromM (0), EMSim::Length::fromM (0))), "First periodicity vector")
      ("periodicity-2", boost::program_options::value<Math::Vector3<EMSim::Length> > ()->default_value (Math::Vector3<EMSim::Length> (EMSim::Length::fromM (0), EMSim::Length::fromM (0), EMSim::Length::fromM (0))), "Second periodicity vector")
      ("gamma", boost::program_options::value<ldouble> ()->default_value (0.001l), "Cutoff parameter for periodic particles")

      ("far-field", boost::program_options::value<std::vector<std::string> > (), "Output far field, see '--far-field help' for more information")
      ("efield", "Output electric far field in YZ plane")
      ("mueller-matrix", "Output mueller matrix in YZ plane")
      ("ntheta", boost::program_options::value<uint32_t> (), "Number of angles for YZ plane efield calculation")
      ("efield-grid", "Output electric far field grid")
      ("mueller-matrix-grid", "Output mueller matrix grid")

      ("fft-grid", boost::program_options::value<Math::Vector3<uint32_t> > ()->default_value (Math::Vector3<uint32_t> (0, 0, 0)), "FFT grid size")
      ("epsilon", boost::program_options::value<ldouble> ()->default_value (5), "Stopping criterion for the solver (use 10^-<value> as stopping criterion)")
      ("maxiter", boost::program_options::value<size_t> ()->default_value (-1), "Maximum number of iterations)")
      ("iter", boost::program_options::value<std::string> ()->default_value ("qmr"), "The iterative algorithm to use (qmr, cgnr, bicg, bicgstab)")

      ("ftype", boost::program_options::value<std::string> ()->default_value ("double"), "Floating point type, can be float, double or ldouble")
      ("cpu", "Run on the CPU")
      ("opencl", "Run with OpenCL")
      ("device", boost::program_options::value<std::string> ()->default_value ("auto"), "Choose the OpenCL device, use `list' to show available devices")
      ("sync", "Sync after every step")
      ("dmatrix-host", "Put DMatrix into Host RAM")
      ("opencl-fft", boost::program_options::value<std::string> (), "The OpenCL FFT implementation to use")

      ("output-dir", boost::program_options::value<std::string> (), "Directory for output files")
      ("output-parent-dir", boost::program_options::value<std::vector<std::string> > (), "Directory for creating the output directory (ignored when --output-dir is given)")
      ("tag", boost::program_options::value<std::vector<std::string> > (), "Tag names for output directory")

      ("mem-info", "Output info about GPU memory usage")
      ("profiling-run", "Measure time needed for one iteration")

      ("load-dip-pol", boost::program_options::value<std::string> (), "Load dipole polarizations from file")
      ("load-start-dip-pol", boost::program_options::value<std::string> (), "Load dipole polarization start values for solver from file")

      ("store-incbeam", "Store incident beam field")
      ("store-intfield", "Store internal field")
      ("store-dippol", "Store dipole polarizations (currently always enabled)")
      ("write-txt", "Store output files as text files")
      ;

#ifdef OPTIONS_ADDITIONAL_OPTIONS
    OPTIONS_ADDITIONAL_OPTIONS
#endif

      descriptionAll_.reset (new Description (description_));
    descriptionAll_->add_options ()
      ("adda-options", "")
      ("positional-argument", boost::program_options::value<std::vector<std::string> > ()->default_value (std::vector<std::string> (), ""))
      ;
  }
  Options::~Options () {
  }

  boost::optional<Options::Map> Options::parse (int argc, const char* const* argv) const {
    boost::program_options::variables_map map;
    try {
      boost::program_options::store (boost::program_options::command_line_parser (argc, const_cast<char**> (argv)).style (style).options (descriptionAll ()).positional (positional ()).run (), map);
    } catch (boost::program_options::error& e) {
      Core::OStream::getStderr ()
        << "Error parsing command line: " << e.what () << std::endl
        << "See 'DDA --help' for more information" << std::endl;
      return boost::none;
    }
    if (!map.count ("adda-options")) {
      if (map["positional-argument"].as<std::vector<std::string> > ().size ()) {
        Core::OStream::getStderr ()
          << "Error parsing command line: too many positional options" << std::endl
          << "See 'DDA --help' for more information" << std::endl;
        return boost::none;
      }
    } else {
      parseAddaOptions (map, map["positional-argument"].as<std::vector<std::string> > ());
      std::map<std::string, boost::program_options::variable_value>& map2 (map);
      map2.erase ("adda-options");
      map2.erase ("positional-argument");
    }
    boost::program_options::notify (map);

    if (boost::any_cast<std::vector<std::string> > (&map["m"].value ())) {
      std::vector<Math::DiagMatrix3<cldouble> > mValues;
      BOOST_FOREACH (const std::string& s, map["m"].as<std::vector<std::string> > ()) {
        Math::DiagMatrix3<cldouble> value;
        EMSim::parse (s, value);
        mValues.push_back (value);
      }
      std::map<std::string, boost::program_options::variable_value>& map2 (map);
      map2["m"].value () = boost::any (mValues);
    }

    return map;
  }

  void Options::help () const {
    Core::OStream::getStdout () << "Usage: DDA [options]" << std::endl << description ();
  }

  std::vector<std::string> Options::getParameters (const Map& map) const {
    std::vector<std::string> ret;
    typedef std::pair<const std::string, boost::program_options::variable_value> pairtype;
    BOOST_FOREACH (const pairtype& pair, map) {
      const boost::program_options::variable_value& v = pair.second;
      const boost::program_options::option_description& d = descriptionAll ().find (pair.first, false);
      if (v.defaulted ())
        continue;
      unsigned min = d.semantic ()->min_tokens ();
      unsigned max = d.semantic ()->max_tokens ();
      ASSERT (min == max);
      if (min > 0) {
        std::vector<std::string> s = anyToStringList (v.value ());
        ASSERT (s.size () != 0);
        BOOST_FOREACH (const std::string& val, s) {
          ret.push_back ("--" + pair.first);
          ret.push_back (val);
        }
      } else {
        ret.push_back ("--" + pair.first);
      }
    }
    return ret;
  }
  std::string Options::getParameterString (const Map& map) const {
    std::stringstream str;
    str << "DDA";
    BOOST_FOREACH (const std::string& s, getParameters (map)) {
      bool needEscape = false;
      BOOST_FOREACH (char c, s) {
        if (!((c >= '0' && c <= '9')
              || (c >= 'A' && c <= 'Z')
              || (c >= 'a' && c <= 'z')
              || c == '-' || c == '_' || c == '+' || c == '.' || c == ':'
              || c == '/'))
          needEscape = true;
      }
      if (needEscape)
        str << " '" << Core::findReplace (s, "'", "'\\''") << "'";
      else
        str << " " << s;
    }
    return str.str ();
  }
}
