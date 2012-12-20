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

#include "FarField.hpp"

#include <Core/ProgressBar.hpp>
#include <Core/StringUtil.hpp>

#include <EMSim/DataFiles.hpp>

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/math/constants/constants.hpp>

#include <iomanip>

namespace EMSim {
  template <class ftype>
  void JonesCalculus<ftype>::storeTxt (const Core::OStream& out, const AngleList& angles, const std::string& name, const std::vector<FarFieldEntry<ftype> >& field, bool storePhi) {
    size_t count = angles.count ();
    ASSERT (field.size () == count);
    out << "#theta ";
    if (storePhi)
      out << "phi ";
    out << name << "per.r " << name << "per.i " << name << "par.r " << name << "par.i\n";
    for (size_t i = 0; i < count; i++) {
      std::pair<ldouble, ldouble> thetaPhi = angles.getThetaPhi (i);
      ftype thetaDeg = static_cast<ftype> (thetaPhi.first) / boost::math::constants::pi<ftype> () * 180;
      ftype phiDeg = static_cast<ftype> (thetaPhi.second) / boost::math::constants::pi<ftype> () * 180;
      std::complex<ftype> per = field[i].perpendicular ();
      std::complex<ftype> par = field[i].parallel ();
    
      out << std::fixed << std::setw (2) << std::setprecision (2) << thetaDeg << " ";
      if (storePhi)
        out << std::fixed << std::setw (2) << std::setprecision (2) << phiDeg << " ";
      out << std::scientific << std::setw (10) << std::setprecision (10)
          << per.real () << " " << per.imag () << " "
          << par.real () << " " << par.imag ()
          << "\n";
    }
  }
  template <class ftype>
  void JonesCalculus<ftype>::storeTxt (const boost::filesystem::path& outputFile, const AngleList& angles, const std::string& name, const std::vector<FarFieldEntry<ftype> >& field, bool storePhi) {
    storeTxt (Core::OStream::open (outputFile), angles, name, field, storePhi);
  }
}

CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, FarFieldEntry)
CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, FarField)
