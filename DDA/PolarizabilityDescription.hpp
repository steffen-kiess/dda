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

#ifndef DDA_POLARIZABILITYDESCRIPTION_HPP_INCLUDED
#define DDA_POLARIZABILITYDESCRIPTION_HPP_INCLUDED

// Implementations of various polarizability descriptions

#include <complex>
#include <vector>

#include <Math/FPTemplateInstances.hpp>

#include <Math/Vector3.hpp>
#include <Math/DiagMatrix3.hpp>

#include <DDA/BeamPolarization.hpp>
#include <DDA/FPConst.hpp>
#include <DDA/Forward.hpp>

#include <boost/shared_ptr.hpp>

namespace DDA {
  template <class ftype>
  class PolarizabilityDescription {
#ifdef SWIG_VISIBILITY_WORKAROUND
  public:
#endif
    typedef std::complex<ftype> ctype;

  public:
    static boost::shared_ptr<const PolarizabilityDescription<ftype> > parsePolDesc (const std::string& s);

    PolarizabilityDescription ();
    virtual ~PolarizabilityDescription ();
    virtual void getCoupleConstants (const DDAParams<ftype>& ddaParams, const Beam<ftype>& beam, BeamPolarization pol, std::vector<Math::DiagMatrix3<ctype> >& cc) const = 0;
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, PolarizabilityDescription)

  namespace PolarizabilityDescriptions {
    // Clausius–Mossotti
    template <class ftype>
    class Cm : public PolarizabilityDescription<ftype> {
#ifdef SWIG_VISIBILITY_WORKAROUND
    public:
#endif
      typedef std::complex<ftype> ctype;
      typedef FPConst<ftype> Const;
    public:
      Cm ();
      virtual ~Cm ();

      virtual void getCoupleConstants (const DDAParams<ftype>& ddaParams, const Beam<ftype>& beam, BeamPolarization pol, std::vector<Math::DiagMatrix3<ctype> >& cc) const;
    };

    CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, Cm)

    // lattice dispersion relation
    // "Beyond Clausius-Mossotti - Wave propagation on a polarizable point lattice and the discrete dipole approximation", B. T. Draine, J. Goodman, doi:10.1086/172396
    template <class ftype>
    class Ldr : public PolarizabilityDescription<ftype> {
#ifdef SWIG_VISIBILITY_WORKAROUND
    public:
#endif
      typedef std::complex<ftype> ctype;
      typedef FPConst<ftype> Const;

      bool avgpol_;
    public:
      Ldr (bool avgpol);
      virtual ~Ldr ();

      bool avgpol () const { return avgpol_; }

      virtual void getCoupleConstants (const DDAParams<ftype>& ddaParams, const Beam<ftype>& beam, BeamPolarization pol, std::vector<Math::DiagMatrix3<ctype> >& cc) const;
    };

    CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, Ldr)
  }
}

#endif // !DDA_POLARIZABILITYDESCRIPTION_HPP_INCLUDED
