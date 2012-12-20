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

#include "PolarizabilityDescription.hpp"

#include <Core/HelpResultException.hpp>

#include <DDA/DDAParams.hpp>
#include <DDA/Beam.hpp>

#include <Math/Math.hpp>

namespace DDA {
  template <class ftype>
  boost::shared_ptr<const PolarizabilityDescription<ftype> > PolarizabilityDescription<ftype>::parsePolDesc (const std::string& s) {
    const std::string& name = s;
    if (name == "help") {
      std::stringstream str;
      str << "Available polarizability descriptions:" << std::endl;
      str << "cm         Clausius–Mossotti" << std::endl;
      str << "ldr        Lattice Dispersion Relation (Draine & Goodman)" << std::endl;
      str << "ldr-avgpol Lattice Dispersion Relation with avgpol " << std::endl;
      throw Core::HelpResultException (str.str ());
    } else if (name == "cm") {
      return boost::shared_ptr<const PolarizabilityDescription<ftype> > (new PolarizabilityDescriptions::Cm<ftype> ());
    } else if (name == "ldr") {
      return boost::shared_ptr<const PolarizabilityDescription<ftype> > (new PolarizabilityDescriptions::Ldr<ftype> (false));
    } else if (name == "ldr-avgpol") {
      return boost::shared_ptr<const PolarizabilityDescription<ftype> > (new PolarizabilityDescriptions::Ldr<ftype> (true));
    } else {
      ABORT_MSG ("Unknown polarizability description `" + name + "'");
    }
  }
  template <class ftype>
  PolarizabilityDescription<ftype>::PolarizabilityDescription () {}
  template <class ftype>
  PolarizabilityDescription<ftype>::~PolarizabilityDescription () {}
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, PolarizabilityDescription)

  namespace PolarizabilityDescriptions {
    template <class ftype>
    Cm<ftype>::Cm () {}
    template <class ftype>
    Cm<ftype>::~Cm () {}
    template <class ftype>
    void Cm<ftype>::getCoupleConstants (const DDAParams<ftype>& ddaParams, UNUSED const Beam<ftype>& beam, UNUSED BeamPolarization pol, std::vector<Math::DiagMatrix3<ctype> >& cc) const {
      ASSERT (ddaParams.dipoleGeometry ().materials ().size () == cc.size ());

      ftype temp = static_cast<ftype> (ddaParams.dipoleGeometry ().gridUnitVol ()) / Const::four_pi;
      for (uint32_t i = 0; i < ddaParams.dipoleGeometry ().materials ().size (); i++) {
        Math::DiagMatrix3<ctype> mat = (Math::DiagMatrix3<ctype>) ddaParams.dipoleGeometry ().materials ()[i];
        for (int j = 0; j < 3; j++) {
          ctype m2 = mat[j] * mat[j];
          ctype uncorrected = 3 * temp * (m2 - ctype (1)) / (m2 + ctype (2));
          cc[i][j] = uncorrected;
        }
      }
    }
    CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, Cm)

    template <class ftype>
    Ldr<ftype>::Ldr (bool avgpol) : avgpol_ (avgpol) {}
    template <class ftype>
    Ldr<ftype>::~Ldr () {}
    template <class ftype>
    void Ldr<ftype>::getCoupleConstants (const DDAParams<ftype>& ddaParams, const Beam<ftype>& beam, BeamPolarization pol, std::vector<Math::DiagMatrix3<ctype> >& cc) const {
      ASSERT (ddaParams.dipoleGeometry ().materials ().size () == cc.size ());

      ftype temp = static_cast<ftype> (ddaParams.dipoleGeometry ().gridUnitVol ()) / Const::four_pi;
      for (uint32_t i = 0; i < ddaParams.dipoleGeometry ().materials ().size (); i++) {
        Math::DiagMatrix3<ctype> mat = (Math::DiagMatrix3<ctype>) ddaParams.dipoleGeometry ().materials ()[i];
        for (int j = 0; j < 3; j++) {
          ctype m2 = mat[j] * mat[j];
          ctype uncorrected = 3 * temp * (m2 - ctype (1)) / (m2 + ctype (2));
          ctype t1 = ctype (0, ftype (2) / 3 * ddaParams.kd () * ddaParams.kd () * ddaParams.kd ());
          ctype S;
          Math::Vector3<ftype> prop = static_cast<Math::Vector3<ftype> > (ddaParams.dipoleGeometry ().orientationInverse () * beam.prop ());
          Math::Vector3<ftype> prop2 (Math::squared (prop.x ()), Math::squared (prop.y ()), Math::squared (prop.z ()));
          Math::Vector3<ftype> incPol = beam.getIncPolP (ddaParams.dipoleGeometry (), pol);
          Math::Vector3<ftype> incPol2 (Math::squared (incPol.x ()), Math::squared (incPol.y ()), Math::squared (incPol.z ()));
          if (avgpol ()) {
            S = 0.5f * (1 - prop2 * prop2);
          } else {
            S = prop2 * incPol2;
          }
          t1 += (Const::ldr_b1 + (Const::ldr_b2 + Const::ldr_b3 * S) * m2) * ddaParams.kd () * ddaParams.kd ();
          ctype corrected = uncorrected / (ctype (1) - (uncorrected / ddaParams.gridUnitVol ()) * t1);
          cc[i][j] = corrected;
        }
      }
    }
    CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, Ldr)
  }
}
