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

#include "Beam.hpp"

#include <Core/Type.hpp>
#include <Core/HelpResultException.hpp>

#include <EMSim/Parse.hpp>

#include <DDA/FieldCalculator.hpp>

namespace DDA {
  template <typename T>
  Beam<T>::Beam (Math::Vector3<ldouble> prop) : 
    prop_ (prop / std::sqrt (prop * prop)) // normalize propagation direction
  {
    // Use the same incident polarizations as adda
    Math::Vector3<ldouble> incPolX;
    Math::Vector3<ldouble> incPolY;
    //if (std::abs (prop_.z ()) >= 1) {
    if (std::abs (prop_.z ()) >= (1 - 1e-6)) { // Workaround numeric problems
      incPolX = Math::Vector3<ldouble> (prop_.z (), 0, 0);
      incPolY = Math::Vector3<ldouble> (0, 1, 0);
    } else {
      ldouble t = std::sqrt (1 - prop_.z () * prop_.z ());
      incPolX = Math::Vector3<ldouble> (prop_.x () * prop_.z () / t, prop_.y () * prop_.z () / t, -t);
      incPolY = Math::Vector3<ldouble> (-prop_.y () / t, prop_.x () / t, 0);
    }
    incPol_[BEAMPOLARIZATION_1] = incPolY;
    incPol_[BEAMPOLARIZATION_2] = incPolX;
  }
  template <typename T>
  Beam<T>::~Beam () {}
  template <typename T>
  NORETURN_ATTRIBUTE T Beam<T>::getPhaseShift (UNUSED const DDAParams<ftype>& ddaParams, UNUSED Math::Vector3<ftype> c) const {
    ABORT_MSG ("Beam " + Core::Type::getName (typeid (this)) + " has no support for periodic targets");
  }
  template <typename T>
  T Beam<T>::extCross (const DDAParams<ftype>& ddaParams, UNUSED FieldCalculator<ftype>& calculator, const std::vector<ctype>& pvec, BeamPolarization beamPolarization) const {
    std::vector<ctype> einc (ddaParams.vecSize ());
    createEInc (ddaParams, beamPolarization, einc);
    T sum = 0;
    for (uint32_t i = 0; i < ddaParams.cnvCount (); i++)
      if (ddaParams.dipoleGeometry ().isValid (i))
        sum += std::imag (ddaParams.get (pvec, i) * conj (ddaParams.get (einc, i)));
    sum *= FPConst<ftype>::four_pi * ddaParams.waveNum ();
    return sum;
  }
  template <typename T>
  boost::shared_ptr<Beam<T> > Beam<T>::parseBeam (Math::Vector3<T> prop, const std::string& s) {
    size_t pos = 0;
    while (pos < s.length () && s[pos] >= 'a' && s[pos] <= 'z')
      pos++;
    std::string name = s.substr (0, pos);
    while (pos < s.length () && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == ':'))
      pos++;
    std::string value = s.substr (pos);
    if (name == "help") {
      std::stringstream str;
      str << "Available beams:" << std::endl;
      str << "plane" << std::endl;
      str << "gaussian:lminus,<waist size>[,(centerx,y,z)]" << std::endl;
      str << "gaussian:davis3,<waist size>[,(centerx,y,z)]" << std::endl;
      str << "gaussian:barton5,<waist size>[,(centerx,y,z)]" << std::endl;
      throw Core::HelpResultException (str.str ());
    } else if (name == "plane") {
      return Beams::PlaneWave<T>::parse (prop, value);
    } else if (name == "gaussian") {
      return Beams::Gaussian<T>::parse (prop, value);
    } else {
      ABORT_MSG ("Unknown beam `" + name + "'");
    }
  }
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, Beam)

  namespace Beams {
    template <typename T>
    boost::shared_ptr<PlaneWave<T> > PlaneWave<T>::parse (Math::Vector3<T> prop, const std::string& s) {
      ASSERT (s == "");
      return boost::shared_ptr<PlaneWave<T> > (new PlaneWave<T> (prop));
    }
    template <typename T>
    PlaneWave<T>::PlaneWave (Math::Vector3<ftype> prop) : Beam<T> (prop) {}
    template <typename T>
    PlaneWave<T>::~PlaneWave () {}
    template <typename T>
    bool PlaneWave<T>::rotSym90 () const {
      return true;
    }
    template <typename T>
    void PlaneWave<T>::createEInc (const DDAParams<ftype>& ddaParams, BeamPolarization beamPolarization, std::vector<ctype>& einc) const {
      assert (einc.size () == ddaParams.cvecSize ());
      Math::Vector3<ftype> propP = ddaParams.dipoleGeometry ().orientationInverse () * prop ();
      Math::Vector3<ctype> pol = getIncPolP (ddaParams.dipoleGeometry (), beamPolarization);
      for (uint32_t i = 0; i < ddaParams.cnvCount (); i++) {
        ddaParams.set (einc, i, std::exp (ctype (0, ddaParams.waveNum () * (ddaParams.dipoleGeometry ().template getDipoleCoordPartRef<ftype> (i) * propP))) * pol);
      }
    }
    template <typename T>
    T PlaneWave<T>::getPhaseShift (const DDAParams<ftype>& ddaParams, Math::Vector3<ftype> c) const {
      Math::Vector3<ftype> k0 = propP (ddaParams.dipoleGeometry ()) * ddaParams.waveNum ();
      return k0 * c;
    }
    // Extinction cross-section, plane beam type
    template <typename T>
    T PlaneWave<T>::extCross (const DDAParams<ftype>& ddaParams, FieldCalculator<ftype>& calculator, const std::vector<ctype>& pvec, BeamPolarization beamPolarization) const {
      calculator.setPVec (pvec);
      Math::Vector3<ftype> propP = ddaParams.dipoleGeometry ().orientationInverse () * prop ();
      Math::Vector3<ctype> ebuff = calculator.calcField (propP);
      ftype sum = -real (ebuff) * getIncPolPF (ddaParams.dipoleGeometry (), beamPolarization);
      sum *= FPConst<ftype>::four_pi / ddaParams.waveNum ();
      return sum;
    }
    CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, PlaneWave)

    template <typename T>
    boost::shared_ptr<Gaussian<T> > Gaussian<T>::parse (Math::Vector3<T> prop, const std::string& s) {
      size_t pos = 0;
      while (pos < s.length () && ((s[pos] >= 'a' && s[pos] <= 'z') || (s[pos] >= '0' && s[pos] <= '9')))
        pos++;
      std::string name = s.substr (0, pos);
      while (pos < s.length () && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == ','))
        pos++;
      std::string value = s.substr (pos);
      pos = 0;
      while (pos < value.length () && value[pos] != ',')
        pos++;
      std::string sizeS, centerS;
      if (pos < value.length ()) {
        sizeS = value.substr (0, pos);
        centerS = value.substr (pos + 1);
      } else {
        sizeS = value;
        centerS = "";
      }
      GaussianBeamType type;
      if (name == "help") {
        std::stringstream str;
        str << "Available gaussian beams:" << std::endl;
        str << "gaussian:lminus,<waist size>[,(x,y,z)]" << std::endl;
        str << "gaussian:davis3,<waist size>[,(x,y,z)]" << std::endl;
        str << "gaussian:barton5,<waist size>[,(x,y,z)]" << std::endl;
        throw Core::HelpResultException (str.str ());
      } else if (name == "lminus") {
        type = GAUSSIANBEAMTYPE_LMINUS;
      } else if (name == "davis3") {
        type = GAUSSIANBEAMTYPE_DAVIS3;
      } else if (name == "barton5") {
        type = GAUSSIANBEAMTYPE_BARTON5;
      } else {
        ABORT_MSG ("Unknown gaussian beam type `" + name + "'");
      }
      EMSim::Length waistSize;
      EMSim::parse (sizeS, waistSize);
      Math::Vector3<EMSim::Length> center (EMSim::Length::zero (), EMSim::Length::zero (), EMSim::Length::zero ());
      if (centerS != "") {
        EMSim::parse (centerS, center);
      }
      return boost::shared_ptr<Gaussian<T> > (new Gaussian<T> (prop, type, waistSize, center));
    }
    template <typename T>
    Gaussian<T>::Gaussian (Math::Vector3<ftype> prop, GaussianBeamType type, EMSim::Length waistSize, Math::Vector3<EMSim::Length> beamCenter) : Beam<T> (prop), type_ (type), waistSize_ (waistSize), beamCenter_ (beamCenter) {}
    template <typename T>
    Gaussian<T>::~Gaussian () {}
    template <typename T>
    bool Gaussian<T>::rotSym90 () const {
      return Math::abs2 (beamCenter_ - (beamCenter_ * prop ()) * prop ()) < 1e-12 * Math::abs2 (beamCenter_ * 1);
    }
    template <typename T>
    void Gaussian<T>::createEInc (const DDAParams<ftype>& ddaParams, BeamPolarization beamPolarization, std::vector<ctype>& einc) const {
      assert (einc.size () == ddaParams.cvecSize ());
      Math::Vector3<ftype> propP = ddaParams.dipoleGeometry ().orientationInverse () * prop ();
      Math::Vector3<ftype> ex = getIncPolP (ddaParams.dipoleGeometry (), beamPolarization);
      Math::Vector3<ftype> ey = beamPolarization == BEAMPOLARIZATION_1 ? -getIncPolP (ddaParams.dipoleGeometry (), BEAMPOLARIZATION_2) : getIncPolP (ddaParams.dipoleGeometry (), BEAMPOLARIZATION_1);
      ftype w0 = waistSize_.valueAs<ftype> ();
      ftype b = ddaParams.waveNum () * w0 * w0; // confocal parameter

      Math::Vector3<ftype> beamCenter (beamCenter_.x ().template valueAs<ftype> (), beamCenter_.y ().template valueAs<ftype> (), beamCenter_.z ().template valueAs<ftype> ());
      Math::Vector3<ftype> centerP = ddaParams.dipoleGeometry ().orientationInverse () * beamCenter; // beam center in particle ref
    
      for (uint32_t i = 0; i < ddaParams.cnvCount (); i++) {
        Math::Vector3<ftype> coord = ddaParams.dipoleGeometry ().template getDipoleCoordPartRef<ftype> (i) - centerP;
        ftype x = coord * ex; // Coordinates of the point in the incident beam system
        ftype y = coord * ey;
        ftype z = coord * propP;
        ftype xi = x / w0; // Normalized coordinates
        ftype eta = y / w0;
        ftype zeta = z / b;
        ftype rho2 = xi * xi + eta * eta; // squared distance from the beam axis
        //ctype Q = FPConst<ftype>::one / ctype (2 * zeta, 1); // 1 / (i + 2zeta) // Version from doi:10.1063/1.344207
        ctype Q = FPConst<ftype>::one / ctype (2 * zeta, -1); // 1 / (-i + 2zeta)
        //ctype psi0 = ctype (0, 1) * Q * std::exp (ctype (0, -rho2) * Q); // i Q exp (-i rho Q) // Version from doi:10.1063/1.344207
        ctype psi0 = ctype (0, -1) * Q * std::exp (ctype (0, rho2) * Q); // -i Q exp (i rho Q)
        ctype phase = std::exp (ctype (0, ddaParams.waveNum () * z));

        //Core::OStream::getStdout () << xi << " " << eta << " " << zeta << " " << Q << " " << psi0 << " " << (psi0 * phase) << std::endl;
      
        Math::Vector3<ctype> value;
        if (type_ == GAUSSIANBEAMTYPE_LMINUS) {
          value = ex * psi0 * phase; // doi:10.1364/JOSAA.5.001427 eq. (22) E_u = E_0 psi_0 exp(-ikw)
        } else if (type_ == GAUSSIANBEAMTYPE_DAVIS3) {
          ftype s = 1 / (ddaParams.waveNum () * w0);
          ftype s2 = s * s;
          ftype s3 = s2 * s;
          ftype rho4 = rho2 * rho2;
          ctype Q2 = Q * Q;
          ctype Q3 = Q2 * Q;
          ctype Q4 = Q2 * Q2;
          ftype xi2 = xi * xi;
          ctype i (0, 1);
          //ctype xFact = ftype (1) + s2 * (-ftype (4) * Q2 * xi2 + i * Q3 * rho4);
          //ctype yFact = 0;
          //ctype zFact = -s * 2 * Q * xi + s3 * (ftype (8) * Q3 * rho2 * xi - ftype (2) * i * Q4 * rho4 * xi + ftype (4) * i * Q2 * xi);
          // Swap signs of imaginary parts
          ctype xFact = ftype (1) + s2 * (-ftype (4) * Q2 * xi2 - i * Q3 * rho4);
          ctype yFact = 0;
          ctype zFact = -s * 2 * Q * xi + s3 * (ftype (8) * Q3 * rho2 * xi + ftype (2) * i * Q4 * rho4 * xi - ftype (4) * i * Q2 * xi);
          value = ex * (xFact * psi0 * phase)
            + ey * (yFact * psi0 * phase)
            + propP * (zFact * psi0 * phase);
        } else if (type_ == GAUSSIANBEAMTYPE_BARTON5) {
          ftype s = 1 / (ddaParams.waveNum () * w0);
          ftype s2 = s * s;
          ftype rho4 = rho2 * rho2;
          ctype Q2 = Q * Q;
          ctype Q3 = Q2 * Q;
          ctype Q4 = Q2 * Q2;
          ctype Q5 = Q4 * Q;
          ftype xi2 = xi * xi;
          ctype i (0, 1);
          //ctype xFact = FPConst<ftype>::one + s2 * (-rho2 * Q2 + i * rho4 * Q3 - ftype (2) * Q2 * xi2) + std::pow (s, FPConst<ftype>::four) * (2 * rho4 * Q4 - ftype (3) * i * rho4 * rho2 * Q5 - ftype (1) / 2 * rho4 * rho4 * Q4 * Q2 + (8 * rho2 * Q4 - i * ftype (2) * rho4 * Q5) * xi2); // doi:10.1063/1.344207 eq. (25)
          //ctype yFact = s2 * (- ftype (2) * Q2 * xi * eta) + s2 * s2 * ((8 * rho2 * Q4 - ftype (2) * i * rho4 * Q5) * xi * eta);
          //ctype zFact = s * (- ftype (2) * Q * xi) + s2 * s * ((+ 6 * rho2 * Q3 - ftype (2) * i * rho4 * Q4) * xi) + s2 * s2 * s * ((-20 * rho4 * Q5 + ftype (10) * i * rho4 * rho2 * Q4 * Q2 + rho4 * rho4 * Q4 * Q3) * xi);
          // Swap signs of imaginary parts
          ctype xFact = FPConst<ftype>::one + s2 * (-rho2 * Q2 - i * rho4 * Q3 - ftype (2) * Q2 * xi2) + std::pow (s, FPConst<ftype>::four) * (2 * rho4 * Q4 + ftype (3) * i * rho4 * rho2 * Q5 - ftype (1) / 2 * rho4 * rho4 * Q4 * Q2 + (8 * rho2 * Q4 + i * ftype (2) * rho4 * Q5) * xi2); // doi:10.1063/1.344207 eq. (25)
          ctype yFact = s2 * (- ftype (2) * Q2 * xi * eta) + s2 * s2 * ((8 * rho2 * Q4 + ftype (2) * i * rho4 * Q5) * xi * eta);
          ctype zFact = s * (- ftype (2) * Q * xi) + s2 * s * ((+ 6 * rho2 * Q3 + ftype (2) * i * rho4 * Q4) * xi) + s2 * s2 * s * ((-20 * rho4 * Q5 - ftype (10) * i * rho4 * rho2 * Q4 * Q2 + rho4 * rho4 * Q4 * Q3) * xi);
          value = ex * (xFact * psi0 * phase)
            + ey * (yFact * psi0 * phase)
            + propP * (zFact * psi0 * phase);
        } else {
          ABORT ();
        }
        ddaParams.set (einc, i, value);
      }
    }
    CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, Gaussian)
  }
}
