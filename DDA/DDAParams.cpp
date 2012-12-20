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

#include "DDAParams.hpp"

#include <DDA/Beam.hpp>

#include <algorithm>

#include <DDA/PolarizabilityDescription.hpp>
#include <DDA/Geometry.hpp>

#include <Math/Vector3IOS.hpp>
#include <Math/DiagMatrix3IOS.hpp>

namespace DDA {
  static const bool use128BitAlignment = true;

  namespace {
    struct CompareVectorZ {
      bool operator() (Math::Vector3<uint32_t> a, Math::Vector3<uint32_t> b) const {
        return a.z () < b.z ();
      }
    };

    cuint32_t fftFit (cuint32_t x, UNUSED uint32_t divis, cuint32_t override, bool supportNonPot) {
      if (override != 0) {
        ASSERT (x <= override);
        return override;
      }

      if (x == 0)
        return 0;

      if (!supportNonPot) {
        cuint32_t ret = 1;
        while (x > 1) {
          ret *= 2;
          x = (x + 1) / 2;
        }
        return ret;
      }

      // Choose fft grid size similar to adda
      cuint32_t ret = x;
      if (ret % 2 == 1)
        ret++;
      for (;;) {
        cuint32_t y = ret;
        while (y % 2 == 0)
          y /= 2;
        while (y % 3 == 0)
          y /= 3;
        while (y % 5 == 0)
          y /= 5;
        while (y % 7 == 0)
          y /= 7;

        /*
          if (y % 11 == 0)
          y /= 11;
          else if (y % 13 == 0)
          y /= 13;
        */

        if (y == 1)
          return ret;

        ret += 2;
      }
    }
  }

  template <class F> DDAParams<F>::DDAParams (const boost::shared_ptr<const Geometry>& geometry,
                                              const std::string& geometryString,
                                              const boost::shared_ptr<const DipoleGeometry>& dipoleGeometry,
                                              ftype lambda,
                                              bool supportNonPot,
                                              cuint32_t procs,
                                              Math::Vector3<cuint32_t> gridSize,
                                              ftype gamma,
                                              const std::vector<uint32_t>& localGridXPar,
                                              const std::vector<uint32_t>& localBoxZPar)
    : geometry_ (geometry),
      geometryString_ (geometryString),
      dipoleGeometry_ (dipoleGeometry),
      lambda_ (lambda), 
      gamma_ (gamma),
      procs_ (procs),
      gridUnit_ (static_cast<ftype> (dipoleGeometry->gridUnit ())),
      periodicity1_ (static_cast<Math::Vector3<ftype> > (dipoleGeometry->periodicity1 ())),
      periodicity2_ (static_cast<Math::Vector3<ftype> > (dipoleGeometry->periodicity2 ())),
      nvCount_ (dipoleGeometry->nvCount ()),
      vecStride_ (roundUp (nvCount_, vecStrideAlign)), // Add padding for coalesced acceses
      vecSize_ (vecStride_ * 3),
      gridSize_ (Math::Vector3<cuint32_t> (fftFit (dipoleGeometry->box ().x () * 2, 1, gridSize.x (), supportNonPot), fftFit (dipoleGeometry->box ().y () * 2, 1, gridSize.y (), supportNonPot), fftFit (dipoleGeometry->box ().z () * 2, 1, gridSize.z (), supportNonPot))),
      localCount_ (procs ()),
      localNvCount_ (procs ()),
      localVecSize_ (procs ()),
      localVecStride_ (procs ()),
      localGridX_ (procs ()),
      localBoxZ_ (procs ()),
      localX0_ (procs ()),
      localZ0_ (procs ()),
      localVec0_ (procs ()),
      localGridXMax_ (0),
      localBlockOffset_ ((procs * (procs + 1)) ())
  {
    ASSERT (lambda > 0);
    ASSERT (procs > 0);

    ASSERT (gridSize_.x () % 2 == 0);
    ASSERT (gridSize_.y () % 2 == 0);
    ASSERT (gridSize_.z () % 2 == 0);

    if (localGridXPar.size () == 0) {
      cuint32_t xRem = cgridX ();
      for (uint32_t i = 0; i < procs; i++) {
        cuint32_t rProc = procs - i;
        cuint32_t val = (xRem + rProc - 1) / rProc;
        localGridX_[i] = val ();
        xRem -= val;
      }
      ASSERT (xRem == 0);
    } else {
      ASSERT (localGridXPar.size () == procs ());
      localGridX_ = localGridXPar;
    }
    {
      cuint32_t xSum = 0;
      cuint32_t max = 0;
      for (uint32_t i = 0; i < procs; i++) {
        localX0_[i] = xSum ();
        xSum += localCGridX (i);
        if (localCGridX (i) > max)
          max = localCGridX (i);
      }
      ASSERT (xSum == cgridX ());
      localGridXMax_ = max;
    }

    if (localBoxZPar.size () == 0) {
      cuint32_t zRem = dipoleGeometry->box ().z ();
      for (uint32_t i = 0; i < procs; i++) {
        cuint32_t rProc = procs - i;
        cuint32_t val = (zRem + rProc - 1) / rProc;
        localBoxZ_[i] = val ();
        zRem -= val;
      }
      ASSERT (zRem == 0);
    } else {
      ASSERT (localBoxZPar.size () == procs ());
      localBoxZ_ = localBoxZPar;
    }
    {
      cuint32_t zSum = 0;
      for (uint32_t i = 0; i < procs; i++) {
        localZ0_[i] = zSum ();
        zSum += localCBoxZ (i);
        localCount_[i] = (localCBoxZ (i) * dipoleGeometry->box ().x () * dipoleGeometry->box ().y ()) ();
      }
      ASSERT (zSum == dipoleGeometry->box ().z ());
    }

    for (uint32_t i = 0; i < procs; i++) {
      cuint32_t sum = 0;
      for (uint32_t j = 0; j < procs; j++) {
        localBlockOffset_[(procs * j + i) ()] = sum ();
        sum += std::max (localCGridX (i) * localCBoxZ (j), localCGridX (j) * localCBoxZ (i)) * dipoleGeometry->box ().y () * 3;
      }
      localBlockOffset_[(procs * procs + i) ()] = sum ();
    }

    {
      cuint32_t nvSum = 0;
      for (uint32_t i = 0; i < procs; i++) {
        std::vector<Math::Vector3<uint32_t> >::const_iterator start = std::lower_bound (this->dipoleGeometry ().positions ().begin (), this->dipoleGeometry ().positions ().end (), Math::Vector3<uint32_t> (0, 0, localZ0 (i)), CompareVectorZ ());
        std::vector<Math::Vector3<uint32_t> >::const_iterator end = std::upper_bound (this->dipoleGeometry ().positions ().begin (), this->dipoleGeometry ().positions ().end (), Math::Vector3<uint32_t> (0, 0, localZ0 (i) + localBoxZ (i) - 1), CompareVectorZ ());
        cuint32_t pos = start - this->dipoleGeometry ().positions ().begin ();
        cuint32_t len = end - start;
        ASSERT (pos == nvSum);
        localVec0_[i] = nvSum ();
        nvSum += len;
        localNvCount_[i] = len ();
        cuint32_t vecStride = (cuint32_t) roundUp (len, vecStrideAlign);
        cuint32_t vecSize = vecStride * 3;
        localVecStride_[i] = vecStride ();
        localVecSize_[i] = vecSize ();
      }
      ASSERT (nvSum == cnvCount ());
    }

    waveNum_ = Const::two_pi / lambda;
    kd_ = waveNum_ * gridUnit ();
  }

  template <class F> std::string DDAParams<F>::toString (const Beam<ftype>& beam, const CoupleConstants<ftype>& cc1, const CoupleConstants<ftype>& cc2) const {
    std::stringstream out;

    out.precision (10);
    out.fill (' ');
    out << std::fixed;

    //geometry ().dump (false, out);
    out << "Processors: " << procs () << std::endl;
    out << std::endl;

    out << "Wavelength: " << (lambda () * 1e+6) << " um" << std::endl;
    out << "Wavenumber: " << (waveNum () * 1e-6) << " 1/um" << std::endl;
    out << std::endl;

    out << "Propagation direction: " << beam.prop () << std::endl;
    out << "Incident polarization 1 (\"y polarization\"): " << beam.getIncPol (BEAMPOLARIZATION_1) << std::endl;
    out << "Incident polarization 2 (\"x polarization\"): " << beam.getIncPol (BEAMPOLARIZATION_2) << std::endl;
    out << std::endl;

    out << "Domains: " << dipoleGeometry ().matCount () << std::endl;
    out << "Refractive indices per domain: 3" << std::endl;
    out << "Refractive indices:" << std::endl;
    out.precision (1);
    for (uint32_t mat = 0; mat < dipoleGeometry ().matCount (); mat++) {
      /*
        if (nComp () > 1)
        out << "(";
        for (uint32_t i = 0; i < nComp (); i++)
        out << std::setw (4) << materials ()[mat][i].real () << "+i" << std::setw (4) << materials ()[mat][i].imag ();
        if (nComp () > 1)
        out << ")";
        out << std::endl;
      */
      out << dipoleGeometry ().materials () [mat] << std::endl;
    }
    out << std::endl;

    out.precision (4);
    out.fill (' ');
    out << std::fixed;

    out << "Particle grid   : " << dipoleGeometry ().box ().x () << ", " << dipoleGeometry ().box ().y () << ", ";
    for (uint32_t i = 0; i < procs (); i++)
      out << (i ? " + " : "") << localCBoxZ (i);
    out << " = " << dipoleGeometry ().box ().z () << std::endl;
    out << "FFT grid        : ";
    for (uint32_t i = 0; i < procs (); i++)
      out << (i ? " + " : "") << localCGridX (i);
    out << " = " << cgridX ();
    out << ", " << cgridY () << ", " << cgridZ ();
    out << std::endl;
    out << "Total dipoles   : ";
    for (uint32_t i = 0; i < procs (); i++)
      out << (i ? " + " : "") << localCCount (i);
    out << " = " << dipoleGeometry ().box ().x () * dipoleGeometry ().box ().y () * dipoleGeometry ().box ().z () << std::endl;
    out << "Non-void dipoles: ";
    for (uint32_t i = 0; i < procs (); i++)
      out << (i ? " + " : "") << localCNvCount (i);
    out << " = " << cnvCount () << std::endl;
    out << "Valid nv dipoles: ";
    out << dipoleGeometry ().validNvCount () << std::endl;
    out << "periodicity dim : " << std::setw (8) << periodicityDimension () << std::endl;
    if (periodicityDimension () >= 1)
      out << "periodicity 1   : " << periodicity1 () << " = " << (periodicity1 () * static_cast<ftype> (1e+6)) << "um" << std::endl;
    if (periodicityDimension () >= 2)
      out << "periodicity 2   : " << periodicity2 () << " = " << (periodicity2 () * static_cast<ftype> (1e+6)) << "um" << std::endl;
    out << "gridspace       : " << std::setw (8) << (gridUnit () * 1e+6) << " um" << std::endl;
    out << "dipvol          : " << std::setw (8) << (gridUnitVol () * 1e+18) << " um³" << std::endl;
    out << "kd              : " << std::setw (8) << kd () << std::endl;
    out << std::endl;
    out << "Particle origin in um: " << (dipoleGeometry ().origin () * static_cast<ftype> (1e+6)) << std::endl;

    out.precision (6);
    out << std::scientific;
    out << "Couple constants for pol 1:" << std::endl;
    for (uint32_t mat = 0; mat < dipoleGeometry ().matCount (); mat++)
      out << cc1.cc ()[mat] << std::endl;
    out << "Square root of couple constants for pol 1:" << std::endl;
    for (uint32_t mat = 0; mat < dipoleGeometry ().matCount (); mat++)
      out << cc1.cc_sqrt ()[mat] << std::endl;
    out << "Normalized inverse susceptibility for pol 1:" << std::endl;
    for (uint32_t mat = 0; mat < dipoleGeometry ().matCount (); mat++)
      out << cc1.chi_inv ()[mat] << std::endl;
    out << std::endl;
    out << "Couple constants for pol 2:" << std::endl;
    for (uint32_t mat = 0; mat < dipoleGeometry ().matCount (); mat++)
      out << cc2.cc ()[mat] << std::endl;
    out << "Square root of couple constants for pol 2:" << std::endl;
    for (uint32_t mat = 0; mat < dipoleGeometry ().matCount (); mat++)
      out << cc2.cc_sqrt ()[mat] << std::endl;
    out << "Normalized inverse susceptibility for pol 2:" << std::endl;
    for (uint32_t mat = 0; mat < dipoleGeometry ().matCount (); mat++)
      out << cc2.chi_inv ()[mat] << std::endl;
    out << std::endl;
    
    return out.str ();
  }

  template <class T>
  CoupleConstants<T>::CoupleConstants (const DDAParams<ftype>& ddaParams, const Beam<ftype>& beam, BeamPolarization pol, const PolarizabilityDescription<ftype>& polDesc)
  : cc_ (ddaParams.dipoleGeometry ().materials ().size ()),
    cc_sqrt_ (ddaParams.dipoleGeometry ().materials ().size ()),
    chi_inv_ (ddaParams.dipoleGeometry ().materials ().size ())
  {
    polDesc.getCoupleConstants (ddaParams, beam, pol, cc_);
    ftype temp = static_cast<ftype> (ddaParams.dipoleGeometry ().gridUnitVol ()) / Const::four_pi;
    for (uint32_t i = 0; i < ddaParams.dipoleGeometry ().materials ().size (); i++) {
      Math::DiagMatrix3<ctype> mat = (Math::DiagMatrix3<ctype>) ddaParams.dipoleGeometry ().materials ()[i];
      for (int j = 0; j < 3; j++) {
        ctype m2 = mat[j] * mat[j];
        cc_sqrt_[i][j] = sqrt (cc_[i][j]);
        chi_inv_[i][j] = Const::one / (temp * (m2 - ctype (1)));
      }
    }
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, DDAParams)
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, CoupleConstants)
}
