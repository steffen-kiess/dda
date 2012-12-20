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

#include "JonesCalculus.hpp"

#include <Core/ProgressBar.hpp>
#include <Core/StringUtil.hpp>

#include <EMSim/DataFiles.hpp>
#include <EMSim/FarField.hpp>

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/math/constants/constants.hpp>

#include <iomanip>

namespace EMSim {
  /*
    Amplitude scattering matrix from Bohren, C. & Huffman "D. Absorption and Scattering of Light by Small Particles":
    (Es_par ) = ... (S2 S3) (Ei_par )
    (Es_perp) = ... (S4 S1) (Ei_perp)
  
    s[0][0] = S11 = S2
    s[0][1] = S12 = S3
    s[1][0] = S21 = S4
    s[1][1] = S22 = S1
  */

  template <class ftype>
  boost::shared_ptr<DataFiles::JonesFarField<ftype> > JonesCalculus<ftype>::computeJonesFarField (const AngleList& angles, const std::vector<FarFieldEntry<ftype> >& field1, const std::vector<FarFieldEntry<ftype> >& field2, ftype frequency) {
    boost::shared_ptr<DataFiles::JonesFarField<ftype> > ptr = boost::make_shared<DataFiles::JonesFarField<ftype> > ();

    size_t count = angles.count ();

    ASSERT (field1.size () == count);
    ASSERT (field2.size () == count);

    ptr->Theta.resize (count);
    ptr->Phi.resize (count);
    ptr->Frequency.resize (1);
    ptr->Frequency[0] = static_cast<double> (frequency);
    ptr->Data = boost::make_shared<boost::multi_array<std::complex<ftype>, 4> > (boost::extents[2][2][count][1], boost::fortran_storage_order ());

    for (size_t i = 0; i < count; i++) {
      std::pair<ldouble, ldouble> thetaPhi = angles.getThetaPhi (i);
      ptr->Theta[i] = static_cast<double> (thetaPhi.first);
      ptr->Phi[i] = static_cast<double> (thetaPhi.second);
      getJonesMatrix (angles, i, field1, field2).store (*ptr->Data, i, 0);
    }

    return ptr;
  }

  template <class ftype>
  static JonesMatrix<ftype> getJonesMatrix (const AngleList& angles, size_t i, const std::vector<FarFieldEntry<ftype> >& f1, const std::vector<FarFieldEntry<ftype> >& f2) {
    std::pair<ldouble, ldouble> thetaPhi = angles.getThetaPhi (i);
    ftype sinPhi = std::sin (static_cast<ftype> (thetaPhi.second));
    ftype cosPhi = std::cos (static_cast<ftype> (thetaPhi.second));

    JonesMatrix<ftype> out;
    out[0][0] = cosPhi * f2[i].parallel () + sinPhi * f1[i].parallel ();
    out[0][1] = sinPhi * f2[i].parallel () - cosPhi * f1[i].parallel ();
    out[1][0] = cosPhi * f2[i].perpendicular () + sinPhi * f1[i].perpendicular ();
    out[1][1] = sinPhi * f2[i].perpendicular () - cosPhi * f1[i].perpendicular ();
    return out;
  }

  template <class ftype>
  void JonesCalculus<ftype>::storeTxt (const Core::OStream& out, const boost::shared_ptr<DataFiles::JonesFarField<ftype> >& farField, bool storePhi) {
    const char* name = "";
    size_t count = farField->Theta.size ();
    ASSERT (farField->Phi.size () == count);
    ASSERT (farField->Frequency.size () == 1);
    ASSERT (farField->Data->shape ()[0] == 2);
    ASSERT (farField->Data->shape ()[1] == 2);
    ASSERT (farField->Data->shape ()[2] == count);
    out << "#theta ";
    if (storePhi)
      out << "phi ";
    out << name << "s1.r " << name << "s1.i " << name << "s2.r " << name << "s2.i "  << name << "s3.r " << name << "s3.i " << name << "s4.r " << name << "s4.i\n";
    for (size_t i = 0; i < count; i++) {
      ftype thetaDeg = static_cast<ftype> (farField->Theta[i]) / boost::math::constants::pi<ftype> () * 180;
      ftype phiDeg = static_cast<ftype> (farField->Phi[i]) / boost::math::constants::pi<ftype> () * 180;
      JonesMatrix<ftype> s = JonesMatrix<ftype>::load (*farField->Data, i, 0);

      out << std::fixed << std::setw (2) << std::setprecision (2) << thetaDeg << " ";
      if (storePhi)
        out << std::fixed << std::setw (2) << std::setprecision (2) << phiDeg << " ";
      out << std::scientific << std::setw (10) << std::setprecision (10)
          << s[1][1].real () << " " << s[1][1].imag () << " "
          << s[0][0].real () << " " << s[0][0].imag () << " "
          << s[0][1].real () << " " << s[0][1].imag () << " "
          << s[1][0].real () << " " << s[1][0].imag ()
          << "\n";
    }
  }
  template <class ftype>
  void JonesCalculus<ftype>::storeTxt (const boost::filesystem::path& outputFile, const boost::shared_ptr<DataFiles::JonesFarField<ftype> >& farField, bool storePhi) {
    storeTxt (Core::OStream::open (outputFile), farField, storePhi);
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, JonesMatrix)
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, JonesCalculus)
}
