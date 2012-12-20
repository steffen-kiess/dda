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

#ifndef EMSIM_JONESCALCULUS_HPP_INCLUDED
#define EMSIM_JONESCALCULUS_HPP_INCLUDED

// Implementation of Jones Matrices and code for creating Jones Matrices

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
  class JonesMatrix {
    typedef std::complex<ftype> SliceType[2];
    std::complex<ftype> values_[2][2];

  public:
    const SliceType& operator[] (size_t i) const { return values_[i]; }
    SliceType& operator[] (size_t i) { return values_[i]; }

    template <typename ftype2>
    static JonesMatrix load (const boost::multi_array<std::complex<ftype2>, 4>& array, size_t pos, size_t fpos) {
      JonesMatrix result;
      for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
          result[i][j] = static_cast<std::complex<ftype> > (array[i][j][pos][fpos]);
      return result;
    }

    template <typename ftype2>
    void store (boost::multi_array<std::complex<ftype2>, 4>& array, size_t pos, size_t fpos) {
      for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
          array[i][j][pos][fpos] = static_cast<std::complex<ftype2> > ((*this)[i][j]);
    }
  };

  template <class ftype>
  class JonesCalculus {
    STATIC_CLASS (JonesCalculus);

  public:
    static boost::shared_ptr<DataFiles::JonesFarField<ftype> > computeJonesFarField (const AngleList& angles, const std::vector<FarFieldEntry<ftype> >& field1, const std::vector<FarFieldEntry<ftype> >& field2, ftype frequency);

    static void storeTxt (const Core::OStream& out, const boost::shared_ptr<DataFiles::JonesFarField<ftype> >& farField, bool storePhi);
    static void storeTxt (const boost::filesystem::path& outputFile, const boost::shared_ptr<DataFiles::JonesFarField<ftype> >& farField, bool storePhi);
    template <typename MethodType, typename GeometryType>
    static void store (const boost::filesystem::path& outputFile, const boost::shared_ptr<GeometryType>& geometry, const boost::shared_ptr<DataFiles::Parameters<MethodType> >& parameters, const boost::shared_ptr<DataFiles::JonesFarField<ftype> >& farField);
  };

  template <typename ftype>
  template <typename MethodType, typename GeometryType>
  inline void JonesCalculus<ftype>::store (const boost::filesystem::path& outputFile, const boost::shared_ptr<GeometryType>& geometry, const boost::shared_ptr<DataFiles::Parameters<MethodType> >& parameters, const boost::shared_ptr<DataFiles::JonesFarField<ftype> >& farField) {
    DataFiles::JonesFarFieldFile<ftype, MethodType, GeometryType> data;

    data.Type = "JonesFarField";
    data.Parameters = parameters;
    data.Geometry = geometry;
    data.JonesFarField = farField;

    HDF5::matlabSerialize (outputFile, data);
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, JonesMatrix)
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, JonesCalculus)
}

#endif // !EMSIM_JONESCALCULUS_HPP_INCLUDED
