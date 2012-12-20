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

#include "Mie.hpp"

#include <Core/Assert.hpp>

#include <EMSim/CrossSection.hpp>
#include <EMSim/AngleList.hpp>
#include <EMSim/DataFiles.hpp>

#include <vector>

#include <boost/make_shared.hpp>
#include <boost/math/constants/constants.hpp>

namespace EMSim {
  typedef ldouble f;
  typedef std::complex<f> c;

  // C++ Translation of the bhmie (C) code by Bohren and Huffman http://code.google.com/p/scatterlib/wiki/Spheres
  void BHMie (ldouble x, cldouble refrel, const AngleList& angleList, ldouble farFieldFactor,
              boost::shared_ptr<DataFiles::JonesFarField<ldouble> >& farField, CrossSection& crossSection, ldouble& qback, ldouble& gsca) {
    size_t count = angleList.count ();
    f& qsca = crossSection.Qsca;

    c cxy = x * refrel;

    // Series expansion terminated after NSTOP terms
    f xstop = x + 4 * std::pow(x, boost::math::constants::third<f> ()) + 2;
    size_t nstop = static_cast<size_t> (xstop);
    f ymod = std::abs (cxy);
    size_t nmx = static_cast<size_t> (std::max (xstop, ymod) + 15);

    size_t nang = count + 2;
    std::vector<f> amu (nang);
    // 0 and pi are needed for scattering cross sections
    amu[0] = 1; // cos (0)
    amu[1] = -1; // cos (pi)
    for (size_t i = 0; i < count; i++)
      amu[2 + i] = std::cos (angleList.getThetaPhi (i).first); // phi is ignored (scattering for a sphere does not depend on phi)

    // Logarithmic derivative D(J) calculated by downward recurrence
    // beginning with initial value (0.,0.) at J=NMX
    std::vector<c> cxd (nmx);
    cxd[nmx - 1] = 0;
    for (size_t i = nmx - 1; i > 0; i--) {
      cxd[i - 1] = f(i + 1) / cxy - c(1) / (cxd[i] + f(i + 1) / cxy);
    }

    std::vector<c> cxs1 (nang);
    std::vector<c> cxs2 (nang);
    std::vector<f> pi0 (nang);
    std::vector<f> pi1 (nang);
    for (size_t i = 0; i < nang; i++) {
      cxs1[i] = cxs2[i] = 0;
      pi0[i] = 0;
      pi1[i] = 1;
    }
    std::vector<f> pi (nang);
    std::vector<f> tau (nang);

    // Riccati-Bessel functions with real argument X
    // calculated by upward recurrence

    f psi0 = std::cos (x);
    f psi1 = std::sin (x);
    f chi0 = -std::sin (x);
    f chi1 = std::cos (x);
    f apsi0 = psi0;
    f apsi1 = psi1;
    c cxxi0 = c (apsi0, -chi0);
    c cxxi1 = c (apsi1, -chi1);
    qsca = 0;
    gsca = 0;
    c cxan;
    c cxan1;
    c cxbn;
    c cxbn1;

    for (size_t n = 0; n < nstop; n++) {
      f rn = n + 1;

      f fn = (2 * rn + 1) / (rn * (rn + 1));
      f psi = (2 * rn - 1) * psi1 / x - psi0;
      f apsi = psi;
      f chi = (2 * rn - 1) * chi1 / x - chi0;
      c cxxi = c (apsi, -chi);
      // Store previous values of AN and BN for use
      // in computation of g=<cos(theta)>
      if (n > 0) {
        cxan1 = cxan;
        cxbn1 = cxbn;
      }

      // Compute AN and BN:
      cxan = ((cxd[n]/refrel + rn/x) * apsi - apsi1) / ((cxd[n]/refrel + rn/x) * cxxi - cxxi1);
      cxbn = ((refrel*cxd[n] + rn/x) * apsi - apsi1) / ((refrel*cxd[n] + rn/x) * cxxi - cxxi1);

      // Augment sums for *qsca and g=<cos(theta)>
      qsca += (2*rn + 1) * (std::norm (cxan) + std::norm (cxbn));
      gsca += ((2*rn + 1) / (rn * (rn+1))) * std::real (cxan * conj (cxbn));

      if (n > 0)
        gsca += ((rn-1) * (rn+1) / rn) * std::real (cxan1 * conj (cxan) + cxbn1 * conj (cxbn));

      for (size_t i = 0; i < nang; i++) {
        pi[i] = pi1[i];
        tau[i] = rn*amu[i]*pi[i] - (rn+1)*pi0[i];
        cxs1[i] += fn * (cxan*pi[i] + cxbn*tau[i]);
        cxs2[i] += fn * (cxan*tau[i] + cxbn*pi[i]);
      }

      psi0 = psi1;
      psi1 = psi;
      apsi1 = psi1;
      chi0 = chi1;
      chi1 = chi;
      cxxi1 = c (apsi1, -chi1);

      // For each angle i, compute pi_n+1 from PI = pi_n , PI0 = pi_n-1
      for (size_t i = 0; i < nang; i++) {
        pi1[i] = ((2*rn + 1) * amu[i] * pi[i] - (rn+1) * pi0[i]) / rn;
        pi0[i] = pi[i];
      }
    }

    // Have summed sufficient terms. Now compute qsca, qext, qback, and gsca
    gsca = 2 * gsca / qsca;
    qsca = (2 / (x*x)) * qsca;
    crossSection.Qext = (4 / (x*x)) * std::real (cxs1[0]);
    qback = (4 / (x*x)) * std::norm (cxs1[1]);

    crossSection.setAbs ();
    crossSection.setCFromQ (0.0/0.0);

    farField = boost::make_shared<DataFiles::JonesFarField<ldouble> > ();
    farField->Theta.resize (count);
    farField->Phi.resize (count);
    farField->Frequency.resize (1);
    farField->Frequency[0] = 0.0 / 0.0;
    farField->Data = boost::make_shared<boost::multi_array<cldouble, 4> > (boost::extents[2][2][count][1], boost::fortran_storage_order ());
    for (size_t i = 0; i < count; i++) {
      std::pair<ldouble, ldouble> thetaPhi = angleList.getThetaPhi (i);
      farField->Theta[i] = static_cast<double> (thetaPhi.first);
      farField->Phi[i] = static_cast<double> (thetaPhi.second);
      // par->par
      (*farField->Data)[0][0][i][0] = cxs2[2 + i] * farFieldFactor;
      // non-diagonal entries are 0
      (*farField->Data)[0][1][i][0] = 0;
      (*farField->Data)[1][0][i][0] = 0;
      // perp->perp
      (*farField->Data)[1][1][i][0] = cxs1[2 + i] * farFieldFactor;
    }
  }
}
