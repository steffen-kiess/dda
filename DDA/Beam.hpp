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

#ifndef DDA_BEAM_HPP_INCLUDED
#define DDA_BEAM_HPP_INCLUDED

#include <Core/Assert.hpp>

#include <Math/Forward.hpp>
#include <Math/Vector3.hpp>

#include <EMSim/Length.hpp>

#include <DDA/Forward.hpp>
#include <DDA/BeamPolarization.hpp>
#include <DDA/DDAParams.hpp>

#include <complex>

#include <boost/shared_ptr.hpp>

// Incident field calculation
//
// Implementations for plane beam and for gaussian beam

namespace DDA {
  template <class T>
  class Beam {
#ifdef SWIG_VISIBILITY_WORKAROUND
  public:
#endif
    typedef T ftype;
    typedef std::complex<ftype> ctype;

  private:
    Math::Vector3<ldouble> prop_;

    Math::Vector3<ldouble> incPol_[BEAMPOLARIZATION_MAX];

  public:
    static boost::shared_ptr<Beam<T> > parseBeam (Math::Vector3<T> prop, const std::string& s);

    Beam (Math::Vector3<ldouble> prop);
    virtual ~Beam ();

    // Returns true if the beam is symmetric to rotation by 90 deg about origin-propagation vector
    virtual bool rotSym90 () const = 0;

    Math::Vector3<ldouble> prop () const { return prop_; }
    Math::Vector3<ldouble> propP (const DipoleGeometry& dipoleGeometry) const {
      return dipoleGeometry.orientationInverse () * prop ();
    }

    virtual void createEInc (const DDAParams<ftype>& ddaParams, BeamPolarization beamPolarization, std::vector<ctype>& einc) const = 0;

    Math::Vector3<ldouble> getIncPol (BeamPolarization beamPolarization) const {
      ASSERT (beamPolarization >= 0 && beamPolarization < BEAMPOLARIZATION_MAX);
      return incPol_[beamPolarization];
    }
    Math::Vector3<ldouble> getIncPolP (const DipoleGeometry& dipoleGeometry, BeamPolarization beamPolarization) const {
      return dipoleGeometry.orientationInverse () * getIncPol (beamPolarization);
    }

    Math::Vector3<ftype> getIncPolPF (const DipoleGeometry& dipoleGeometry, BeamPolarization beamPolarization) const {
      return static_cast<Math::Vector3<ftype> > (getIncPolP (dipoleGeometry, beamPolarization));
    }

    virtual ftype getPhaseShift (const DDAParams<ftype>& ddaParams, Math::Vector3<ftype> c) const;

    // Extinction cross-section
    virtual ftype extCross (const DDAParams<ftype>& ddaParams, FieldCalculator<ftype>& calculator, const std::vector<ctype>& pvec, BeamPolarization beamPolarization) const;
  };

  namespace Beams {
    template <class T>
    class PlaneWave : public Beam<T> {
#ifdef SWIG_VISIBILITY_WORKAROUND
    public:
#endif
      typedef T ftype;
      typedef std::complex<ftype> ctype;

    private:
      using Beam<T>::prop;
      using Beam<T>::propP;
      using Beam<T>::getIncPolP;
      using Beam<T>::getIncPolPF;

    public:
      static boost::shared_ptr<PlaneWave<T> > parse (Math::Vector3<T> prop, const std::string& s);

      PlaneWave (Math::Vector3<ftype> prop);
      virtual ~PlaneWave ();

      virtual bool rotSym90 () const;

      virtual void createEInc (const DDAParams<ftype>& ddaParams, BeamPolarization beamPolarization, std::vector<ctype>& einc) const;

      virtual ftype getPhaseShift (const DDAParams<ftype>& ddaParams, Math::Vector3<ftype> c) const;

      // Extinction cross-section
      virtual ftype extCross (const DDAParams<ftype>& ddaParams, FieldCalculator<ftype>& calculator, const std::vector<ctype>& pvec, BeamPolarization beamPolarization) const;
    };

    // Gaussian beam
    // Lminus: "Light scattering from a sphere arbitrarily located in a Gaussian beam, using a Bromwich formulation", G. Gouesbet, B. Maheu, and G. Gréhan, doi:10.1364/JOSAA.5.001427
    // Davis3: "Theory of electromagnetic beams", L. W. Davis, doi:10.1103/PhysRevA.19.1177
    // Barton5: "Fifth‐order corrected electromagnetic field components for a fundamental Gaussian beam" by J. P. Barton and D. R. Alexander, doi:10.1063/1.344207
    enum GaussianBeamType { GAUSSIANBEAMTYPE_LMINUS, GAUSSIANBEAMTYPE_DAVIS3, GAUSSIANBEAMTYPE_BARTON5 };
    template <class T>
    class Gaussian : public Beam<T> {
#ifdef SWIG_VISIBILITY_WORKAROUND
    public:
#endif
      typedef T ftype;
      typedef std::complex<ftype> ctype;

    private:
      using Beam<T>::prop;
      using Beam<T>::propP;
      using Beam<T>::getIncPolP;
      using Beam<T>::getIncPolPF;

      GaussianBeamType type_;
      EMSim::Length waistSize_;
      Math::Vector3<EMSim::Length> beamCenter_; // Beam center in laboratory ref

    public:
      static boost::shared_ptr<Gaussian<T> > parse (Math::Vector3<T> prop, const std::string& s);

      Gaussian (Math::Vector3<ftype> prop, GaussianBeamType type, EMSim::Length waistSize, Math::Vector3<EMSim::Length> beamCenter);
      virtual ~Gaussian ();

      virtual bool rotSym90 () const;

      virtual void createEInc (const DDAParams<ftype>& ddaParams, BeamPolarization beamPolarization, std::vector<ctype>& einc) const;
    };
  }
}

#endif // !DDA_BEAM_HPP_INCLUDED
