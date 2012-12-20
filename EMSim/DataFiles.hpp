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

#ifndef EMSIM_DATAFILES_HPP_INCLUDED
#define EMSIM_DATAFILES_HPP_INCLUDED

// This are structures representing the method-independent data written
// to the output HDF5 files.
// These structures are written using the matlab HDF5 serialization code.

#include <Core/OStream.forward.hpp>

#include <Math/Vector3.hpp>
#include <Math/DiagMatrix3.hpp>
#include <Math/Quaternion.hpp>
#include <Math/Float.hpp>

#include <HDF5/Matlab.hpp>
#include <HDF5/MultiArray.hpp>

#include <boost/multi_array.hpp>
#include <boost/optional.hpp>

#include <complex>

#include <stdint.h>

namespace EMSim {
  namespace DataFiles {
    struct File {
      std::string Type;

#define MEMBERS(m)                              \
      m (Type)
      HDF5_MATLAB_DECLARE_TYPE (File, MEMBERS)
#undef MEMBERS
    };

    struct MieParameters {
      ldouble Frequency; // in Hz

#define MEMBERS(m)                              \
      m (Frequency)
      HDF5_MATLAB_DECLARE_TYPE (MieParameters, MEMBERS)
#undef MEMBERS
    };

    template <typename MethodType>
    struct Parameters {
      std::string Method; // = "DDA" / "Mie" / ...

      MethodType MethodParameters;

      std::string GeometryString;

      std::string CmdLine;

      Math::DiagMatrix3<ldouble> RefractiveIndexMedium; // refractive index of the medium, currently always 1.0

      Math::Vector3<ldouble> PropagationVector;
      std::string BeamString;
      std::vector<Math::Vector3<ldouble> > Polarizations;

#define MEMBERS(m)                              \
      m (Method)                                \
      m (MethodParameters)                      \
      m (GeometryString)                        \
      m (CmdLine)                               \
      m (RefractiveIndexMedium)                 \
      m (PropagationVector)                     \
      m (BeamString)                            \
      m (Polarizations)
      HDF5_MATLAB_DECLARE_TYPE (Parameters, MEMBERS)
#undef MEMBERS
    };

    template <typename ftype>
    struct JonesFarField {
      // size n, angle in rad
      std::vector<double> Theta;
      std::vector<double> Phi;

      // size m, frequency in Hz
      std::vector<double> Frequency;

      // Jones Matrices, fortran storage order, size 2*2*n*m, in m
      // E(r) = 1 / |r| * exp (ik|r|) * Data[...]
      boost::shared_ptr<boost::multi_array<std::complex<ftype>, 4> > Data;

#define MEMBERS(m)                              \
      m (Theta)                                 \
      m (Phi)                                   \
      m (Frequency)                             \
      m (Data)
      HDF5_MATLAB_DECLARE_TYPE (JonesFarField, MEMBERS)
#undef MEMBERS

      //FarField () : Data (boost::extents[2][2][0], boost::fortran_storage_order ()) {}
    };

    template <typename ftype>
    struct MuellerFarField {
      // size n, angle in rad
      std::vector<ldouble> Theta;
      std::vector<ldouble> Phi;

      // size m, frequency in Hz
      std::vector<double> Frequency;

      // Mueller Matrices, fortran storage order, size 4*4*n*m, in m^2
      boost::shared_ptr<boost::multi_array<ftype, 4> > Data;

#define MEMBERS(m)                              \
      m (Theta)                                 \
      m (Phi)                                   \
      m (Frequency)                             \
      m (Data)
      HDF5_MATLAB_DECLARE_TYPE (MuellerFarField, MEMBERS)
#undef MEMBERS

      //MuellerFarField () : Data (boost::extents[4][4][0], boost::fortran_storage_order ()) {}
    };

    struct MieGeometry {
      std::string Type; // = "Mie"

      ldouble Radius;
      Math::DiagMatrix3<cldouble > RefractiveIndex; // complex refractive index for the sphere

#define MEMBERS(m)                              \
      m (Type)                                  \
      m (Radius)                                \
      m (RefractiveIndex)
      HDF5_MATLAB_DECLARE_TYPE (MieGeometry, MEMBERS)
#undef MEMBERS
    };

    template <typename ftype, typename MethodType, typename GeometryType>
    struct JonesFarFieldFile {
      std::string Type; // = "JonesFarField"

      boost::shared_ptr<DataFiles::Parameters<MethodType> > Parameters;
      boost::shared_ptr<GeometryType> Geometry;
      boost::shared_ptr<DataFiles::JonesFarField<ftype> > JonesFarField;

#define MEMBERS(m)                              \
      m (Type)                                  \
      m (Parameters)                            \
      m (Geometry)                              \
      m (JonesFarField)
      HDF5_MATLAB_DECLARE_TYPE (JonesFarFieldFile, MEMBERS)
#undef MEMBERS
    };

    template <typename ftype, typename MethodType, typename GeometryType>
    struct MuellerFarFieldFile {
      std::string Type; // = "MuellerFarField"

      boost::shared_ptr<DataFiles::Parameters<MethodType> > Parameters;
      boost::shared_ptr<GeometryType> Geometry;
      boost::shared_ptr<DataFiles::MuellerFarField<ftype> > MuellerFarField;

#define MEMBERS(m)                              \
      m (Type)                                  \
      m (Parameters)                            \
      m (Geometry)                              \
      m (MuellerFarField)
      HDF5_MATLAB_DECLARE_TYPE (MuellerFarFieldFile, MEMBERS)
#undef MEMBERS
    };

    struct CrossSection {
      ldouble Cext, Cabs, Csca;
      ldouble Qext, Qabs, Qsca;

#define MEMBERS(m)                              \
      m (Cext)  m (Cabs)  m(Csca)               \
      m (Qext)  m (Qabs)  m(Qsca)
      HDF5_MATLAB_DECLARE_TYPE (CrossSection, MEMBERS)
#undef MEMBERS
    };

    struct CrossSectionFile {
      std::string Type; // = "CrossSection"

      uint32_t BeamPolarization;
      boost::shared_ptr<DataFiles::CrossSection> CrossSection;

#define MEMBERS(m)                              \
      m (Type)                                  \
      m (BeamPolarization)                      \
      m (CrossSection)
      HDF5_MATLAB_DECLARE_TYPE (CrossSectionFile, MEMBERS)
#undef MEMBERS

      void writeTxt (const Core::OStream& out) const;
      void write (const boost::filesystem::path& basename, boost::optional<std::string> txtExt = boost::none, boost::optional<std::string> hdf5Ext = (std::string) ".hdf5") const;
    };
  }
}

#endif // !EMSIM_DATAFILES_HPP_INCLUDED
