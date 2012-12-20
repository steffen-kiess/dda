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

#ifndef EMSIM_MUELLERCALCULUS_HPP_INCLUDED
#define EMSIM_MUELLERCALCULUS_HPP_INCLUDED

// Implementation of Mueller Matrices and code for creating Mueller Matrices

#include <Core/OStream.hpp>

#include <Math/FPTemplateInstances.hpp>

#include <EMSim/Forward.hpp>
#include <EMSim/DataFiles.hpp>

#include <HDF5/Matlab.hpp>

#include <complex>

namespace EMSim {
  template <class ftype>
  class MuellerMatrix {
    typedef ftype SliceType[4];
    ftype values_[4][4];

  public:
    const SliceType& operator[] (size_t i) const { return values_[i]; }
    SliceType& operator[] (size_t i) { return values_[i]; }

    template <typename ftype2>
    static MuellerMatrix load (const boost::multi_array<ftype2, 4>& array, size_t pos, size_t fpos) {
      MuellerMatrix result;
      for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
          result[i][j] = static_cast<ftype> (array[i][j][pos][fpos]);
      return result;
    }

    template <typename ftype2>
    void store (boost::multi_array<ftype2, 4>& array, size_t pos, size_t fpos) {
      for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
          array[i][j][pos][fpos] = static_cast<ftype2> ((*this)[i][j]);
    }
  };

  template <class ftype>
  class MuellerCalculus {
    STATIC_CLASS (MuellerCalculus);

  public:
    static MuellerMatrix<ftype> computeMuellerMatrix (const JonesMatrix<ftype>& s);

    static boost::shared_ptr<DataFiles::MuellerFarField<ftype> > computeMuellerFarField (const boost::shared_ptr<DataFiles::JonesFarField<ftype> >& farField);

    static void storeTxt (const Core::OStream& out, const boost::shared_ptr<DataFiles::MuellerFarField<ftype> >& matrix, bool noPhi);
    static void storeTxt (const boost::filesystem::path& outputFile, const boost::shared_ptr<DataFiles::MuellerFarField<ftype> >& matrix, bool noPhi);
    template <typename MethodType, typename GeometryType>
    static void store (const boost::filesystem::path& outputFile, const boost::shared_ptr<GeometryType>& geometry, const boost::shared_ptr<DataFiles::Parameters<MethodType> >& parameters, const boost::shared_ptr<DataFiles::MuellerFarField<ftype> >& matrix);
  };

  template <typename ftype>
  template <typename MethodType, typename GeometryType>
  inline void MuellerCalculus<ftype>::store (const boost::filesystem::path& outputFile, const boost::shared_ptr<GeometryType>& geometry, const boost::shared_ptr<DataFiles::Parameters<MethodType> >& parameters, const boost::shared_ptr<DataFiles::MuellerFarField<ftype> >& matrix) {
    DataFiles::MuellerFarFieldFile<ftype, MethodType, GeometryType> data;

    data.Type = "MuellerFarField";
    data.Parameters = parameters;
    data.Geometry = geometry;
    data.MuellerFarField = matrix;

    HDF5::matlabSerialize (outputFile, data);
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, MuellerMatrix)
  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, MuellerCalculus)
}

#endif // !EMSIM_MUELLERCALCULUS_HPP_INCLUDED
