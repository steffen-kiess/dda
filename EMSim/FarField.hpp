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

#ifndef EMSIM_FARFIELD_HPP_INCLUDED
#define EMSIM_FARFIELD_HPP_INCLUDED

// Far field values for a specific incident polarization

#include <Core/OStream.hpp>

#include <Math/FPTemplateInstances.hpp>

#include <EMSim/AngleList.hpp>
#include <EMSim/Forward.hpp>
#include <EMSim/DataFiles.hpp>

#include <HDF5/Matlab.hpp>

#include <complex>

#include <boost/multi_array.hpp>

namespace EMSim {
  template <class ftype>
  class FarFieldEntry {
    std::complex<ftype> perpendicular_;
    std::complex<ftype> parallel_;

  public:
    FarFieldEntry () {}
    FarFieldEntry (std::complex<ftype> perpendicular, std::complex<ftype> parallel) : perpendicular_ (perpendicular), parallel_ (parallel) {}

    std::complex<ftype> perpendicular () const { return perpendicular_; }
    std::complex<ftype>& perpendicular () { return perpendicular_; }
    std::complex<ftype> parallel () const { return parallel_; }
    std::complex<ftype>& parallel () { return parallel_; }
  };

  template <class ftype>
  class FarField {
    STATIC_CLASS (FarField);

  public:
    static void storeTxt (const Core::OStream& out, const AngleList& angles, const std::string& name, const std::vector<FarFieldEntry<ftype> >& field, bool storePhi);
    static void storeTxt (const boost::filesystem::path& outputFile, const AngleList& angles, const std::string& name, const std::vector<FarFieldEntry<ftype> >& field, bool storePhi);
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, FarFieldEntry)
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, FarField)
}

#endif // !EMSIM_FARFIELD_HPP_INCLUDED
