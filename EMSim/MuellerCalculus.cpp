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

#include "MuellerCalculus.hpp"

#include <EMSim/DataFiles.hpp>
#include <EMSim/JonesCalculus.hpp>

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/math/constants/constants.hpp>

#include <iomanip>

namespace EMSim {
  template <class ftype>
  MuellerMatrix<ftype> MuellerCalculus<ftype>::computeMuellerMatrix (const JonesMatrix<ftype>& s) {
    MuellerMatrix<ftype> result;

    result[0][0] = (std::norm (s[1][1]) + std::norm (s[0][0]) + std::norm (s[0][1]) + std::norm (s[1][0])) / 2;
    result[0][1] = (-std::norm (s[1][1]) + std::norm (s[0][0]) - std::norm (s[0][1]) + std::norm (s[1][0])) / 2;
    result[0][2] = std::real (s[0][0] * std::conj (s[0][1]) + s[1][1] * std::conj (s[1][0]));
    result[0][3] = std::imag (s[0][0] * std::conj (s[0][1]) - s[1][1] * std::conj (s[1][0]));

    result[1][0] = (-std::norm (s[1][1]) + std::norm (s[0][0]) + std::norm (s[0][1]) - std::norm (s[1][0])) / 2;
    result[1][1] = (std::norm (s[1][1]) + std::norm (s[0][0]) - std::norm (s[0][1]) - std::norm (s[1][0])) / 2;
    result[1][2] = std::real (s[0][0] * std::conj (s[0][1]) - s[1][1] * std::conj (s[1][0]));
    result[1][3] = std::imag (s[0][0] * std::conj (s[0][1]) + s[1][1] * std::conj (s[1][0]));

    result[2][0] = std::real (s[0][0] * std::conj (s[1][0]) + s[1][1] * std::conj (s[0][1]));
    result[2][1] = std::real (s[0][0] * std::conj (s[1][0]) - s[1][1] * std::conj (s[0][1]));
    result[2][2] = std::real (s[1][1] * std::conj (s[0][0]) + s[0][1] * std::conj (s[1][0]));
    result[2][3] = std::imag (s[0][0] * std::conj (s[1][1]) + s[1][0] * std::conj (s[0][1]));

    result[3][0] = std::imag (s[1][0] * std::conj (s[0][0]) + s[1][1] * std::conj (s[0][1]));
    result[3][1] = std::imag (s[1][0] * std::conj (s[0][0]) - s[1][1] * std::conj (s[0][1]));
    result[3][2] = std::imag (s[1][1] * std::conj (s[0][0]) - s[0][1] * std::conj (s[1][0]));
    result[3][3] = std::real (s[1][1] * std::conj (s[0][0]) - s[0][1] * std::conj (s[1][0]));

    return result;
  }

  template <class ftype>
  boost::shared_ptr<DataFiles::MuellerFarField<ftype> > MuellerCalculus<ftype>::computeMuellerFarField (const boost::shared_ptr<DataFiles::JonesFarField<ftype> >& farField) {
    boost::shared_ptr<DataFiles::MuellerFarField<ftype> > ptr = boost::make_shared<DataFiles::MuellerFarField<ftype> > ();

    size_t count = farField->Theta.size ();
    size_t fcount = farField->Frequency.size ();

    ptr->Theta.resize (count);
    ptr->Phi.resize (count);
    ptr->Frequency.resize (fcount);
    for (size_t j = 0; j < fcount; j++)
      ptr->Frequency[j] = farField->Frequency[j];
    ptr->Data = boost::make_shared<boost::multi_array<ftype, 4> > (boost::extents[4][4][count][fcount], boost::fortran_storage_order ());

    for (size_t i = 0; i < count; i++) {
      ptr->Theta[i] = farField->Theta[i];
      ptr->Phi[i] = farField->Phi[i];
      for (size_t j = 0; j < fcount; j++) {
        computeMuellerMatrix (JonesMatrix<ftype>::load (*farField->Data, i, j)).store (*ptr->Data, i, j);
      }
    }

    return ptr;
  }

  template <class ftype>
  void MuellerCalculus<ftype>::storeTxt (const Core::OStream& out, const boost::shared_ptr<DataFiles::MuellerFarField<ftype> >& farField, bool noPhi) {
    const char* name = "";
    size_t count = farField->Theta.size ();
    ASSERT (farField->Phi.size () == count);
    ASSERT (farField->Frequency.size () == 1);
    ASSERT (farField->Data->shape ()[0] == 4);
    ASSERT (farField->Data->shape ()[1] == 4);
    ASSERT (farField->Data->shape ()[2] == count);
    out << "#theta ";
    if (!noPhi)
      out << "phi ";
    out << name << "s11 " << name << "s12 " << name << "s13 " << name << "s14 " << name << "s21 " << name << "s22 " << name << "s23 " << name << "s24 " << name << "s31 " << name << "s32 " << name << "s33 " << name << "s34 " << name << "s41 " << name << "s42 " << name << "s43 " << name << "s44\n";
    for (size_t i = 0; i < count; i++) {
      ftype thetaDeg = static_cast<ftype> (farField->Theta[i]) / boost::math::constants::pi<ftype> () * 180;
      ftype phiDeg = static_cast<ftype> (farField->Phi[i]) / boost::math::constants::pi<ftype> () * 180;

      MuellerMatrix<ftype> entries = MuellerMatrix<ftype>::load (*farField->Data, i, 0);

      out << std::fixed << std::setw (2) << std::setprecision (2) << thetaDeg << " ";
      if (!noPhi)
        out << std::fixed << std::setw (2) << std::setprecision (2) << phiDeg << " ";
      out << std::scientific << std::setw (10) << std::setprecision (10)
          << entries[0][0] << " " << entries[0][1] << " " << entries[0][2] << " " << entries[0][3] << " "
          << entries[1][0] << " " << entries[1][1] << " " << entries[1][2] << " " << entries[1][3] << " "
          << entries[2][0] << " " << entries[2][1] << " " << entries[2][2] << " " << entries[2][3] << " "
          << entries[3][0] << " " << entries[3][1] << " " << entries[3][2] << " " << entries[3][3]
          << "\n";
    }
  }

  template <class ftype>
  void MuellerCalculus<ftype>::storeTxt (const boost::filesystem::path& outputFile, const boost::shared_ptr<DataFiles::MuellerFarField<ftype> >& farField, bool noPhi) {
    storeTxt (Core::OStream::open (outputFile), farField, noPhi);
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, MuellerMatrix)
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, MuellerCalculus)
}
