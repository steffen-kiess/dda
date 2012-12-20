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

#include "AddaOptions.hpp"

#include <EMSim/Parse.hpp>
#include <EMSim/Length.hpp>

#include <DDA/FPConst.hpp>
#include <DDA/ToString.hpp>

#include <Core/Assert.hpp>

#include <string>
#include <vector>
#include <set>

#include <Math/DiagMatrix3.hpp>

namespace DDA {
  static bool isOption (const std::string& s) {
    if (s.length () < 2)
      return false;
    if (s[0] != '-')
      return false;
    char c = s[1];
    return (c >= 'A' && c <= 'Z')
      || (c >= 'a' && c <= 'z');
  }

  template <typename T>
  static void option (boost::program_options::variables_map& map, const std::string& name, T value) {
    std::map<std::string, boost::program_options::variable_value>& map2 (map);
    //map[name].value () = boost::any (value);
    map2[name] = boost::program_options::variable_value (boost::any (value), false);
  }

  void parseAddaOptions (boost::program_options::variables_map& map, const std::vector<std::string>& args) {
    std::set<std::string> hadOption;

    ldouble dpl = 0.0/0.0;
    ldouble lambda = FPConst<ldouble>::two_pi;

    //bool muel = true, ampl = false, grid = false;
    bool muel = true, ampl = true, grid = false; // differs from adda

    std::vector<Math::DiagMatrix3<cldouble > > mValues;
  
    for (size_t i = 0; i < args.size (); i++) {
      ASSERT (isOption (args[i]));
      std::string name = args[i].substr (1);
      std::vector<std::string> pars;
      while (i + 1 < args.size () && !isOption (args[i + 1])) {
        i++;
        pars.push_back (args[i]);
      }

      if (hadOption.find (name) != hadOption.end ()) {
        ABORT_MSG ("Got adda option `" + name + "' more than once");
      }
      hadOption.insert (name);

      /*
        Core::OStream::getStdout () << "Option: `" << name << "' " << pars.size () << std::endl;
        for (size_t j = 0; j < pars.size (); j++)
        Core::OStream::getStdout () << "  " << pars[j] << std::endl;
      */

      if (name == "dpl") {
        ASSERT (pars.size () == 1);
        EMSim::parse (pars[0], dpl);
      } else if (name == "lambda") {
        ASSERT (pars.size () == 1);
        EMSim::parse (pars[0], lambda);
        option (map, "lambda", EMSim::Length::fromMicroM (lambda));
      } else if (name == "pol") {
        ASSERT (pars.size () == 1 || pars.size () == 2);
        std::string s = pars[0];
        if (pars.size () > 1)
          s += "-" + pars[1];
        option (map, "pol", s);
      } else if (name == "beam") {
        ASSERT (pars.size () >= 1);
        if (pars[0] == "plane") {
          ASSERT (pars.size () == 1);
          option<std::string> (map, "beam", "plane");
        } else {
          ASSERT (pars.size () == 2 || pars.size () == 5);
          Math::Vector3<ldouble> center (0, 0, 0);
          if (pars.size () > 2) {
            EMSim::parse (pars[2], center.x ());
            EMSim::parse (pars[3], center.y ());
            EMSim::parse (pars[4], center.z ());
          }
          Math::Vector3<EMSim::Length> centerL (EMSim::Length::fromMicroM (center.x ()), EMSim::Length::fromMicroM (center.y ()), EMSim::Length::fromMicroM (center.z ()));
          option<std::string> (map, "beam", "gaussian:" + pars[0] + "," + pars[1] + "um," + boost::lexical_cast<std::string> (centerL));
        }
      } else if (name == "prop") {
        ASSERT (pars.size () == 3);
        Math::Vector3<ldouble> prop;
        EMSim::parse (pars[0], prop.x ());
        EMSim::parse (pars[1], prop.y ());
        EMSim::parse (pars[2], prop.z ());
        option (map, "prop", prop);
      } else if (name == "orient") {
        ASSERT (pars.size () == 3);
        Math::Vector3<ldouble> orient;
        EMSim::parse (pars[0], orient.x ());
        EMSim::parse (pars[1], orient.y ());
        EMSim::parse (pars[2], orient.z ());
        option (map, "orient", orient);
      } else if (name == "m") {
        ASSERT (pars.size () > 0);
        ASSERT (pars.size () % 2 == 0);
        for (size_t i = 0; i < pars.size (); i += 2) {
          ldouble real, imag;
          EMSim::parse (pars[i], real);
          EMSim::parse (pars[i + 1], imag);
          mValues.push_back (Math::DiagMatrix3<cldouble > (cldouble (real, imag)));
        }
      } else if (name == "shape") {
        ASSERT (pars.size () > 0);
        if (pars[0] == "read") {
          ASSERT (pars.size () == 2);
          option (map, "geometry", "load:file=" + pars[1]);
        } else {
          ABORT_MSG ("Unknown shape type `" + pars[0] + "', only 'read' is known");
        }
      } else if (name == "store_scat_grid") {
        ASSERT (pars.size () == 0);
        grid = true;
      } else if (name == "store_beam") {
        ASSERT (pars.size () == 0);
        option (map, "store-incbeam", 1);
      } else if (name == "store_int_field") {
        ASSERT (pars.size () == 0);
        option (map, "store-intfield", 1);
      } else if (name == "store_dip_pol") {
        ASSERT (pars.size () == 0);
        option (map, "store-dippol", 1);
      } else if (name == "scat_matr") {
        ASSERT (pars.size () == 1);
        if (pars[0] == "muel") {
          muel = true;
          ampl = false;
        } else if (pars[0] == "ampl") {
          muel = false;
          ampl = true;
        } else if (pars[0] == "both") {
          muel = ampl = true;
        } else if (pars[0] == "none") {
          muel = ampl = false;
        } else {
          ABORT_MSG ("Invalid value fot -scat_matr `" + pars[0] + "'");
        }
      } else if (name == "eps") {
        ASSERT (pars.size () == 1);
        ldouble eps;
        EMSim::parse (pars[0], eps);
        option (map, "epsilon", eps);
      } else if (name == "iter") {
        ASSERT (pars.size () == 1);
        option (map, "iter", pars[0]);
      } else if (name == "maxiter") {
        ASSERT (pars.size () == 1);
        int64_t maxiter;
        EMSim::parse (pars[0], maxiter);
        option (map, "maxiter", maxiter);
      } else if (name == "ntheta") {
        ASSERT (pars.size () == 1);
        uint32_t ntheta;
        EMSim::parse (pars[0], ntheta);
        option (map, "ntheta", 2 * ntheta);
      } else if (name == "dir") {
        ASSERT (pars.size () == 1);
        option (map, "output-dir", pars[0]);
      } else if (name == "dda-cpu") {
        ASSERT (pars.size () == 0);
        option (map, "cpu", 1);
      } else if (name == "dda-ftype") {
        ASSERT (pars.size () == 1);
        option (map, "ftype", pars[0]);
      } else if (name == "dda-device") {
        ASSERT (pars.size () == 1);
        option (map, "device", pars[0]);
      } else {
        ABORT_MSG ("Unknown adda option `" + name + "'");
      }
    }

    ASSERT_MSG (hadOption.find ("dpl") != hadOption.end (), "No dpl option given");
    option (map, "grid-unit", EMSim::Length::fromMicroM (lambda / dpl));

    option (map, "write-txt", 1);

    if (mValues.size () == 0)
      mValues.push_back (Math::DiagMatrix3<cldouble > (1.5));

    if (hadOption.find ("shape") == hadOption.end ()) {
      ASSERT (mValues.size () == 1);
      ldouble diameter = lambda / dpl * 16;
      ldouble radius = diameter / 2;
      option<std::string> (map, "geometry", "sphere:" + toString (radius) + "um," + toString (mValues[0]));
    } else {
      option (map, "m", mValues);
    }

    if (grid) {
      if (muel)
        option (map, "mueller-matrix-grid", 1);
      if (ampl)
        option (map, "efield-grid", 1);
    } else {
      if (muel)
        option (map, "mueller-matrix", 1);
      if (ampl)
        option (map, "efield", 1);
    }
  }
}
