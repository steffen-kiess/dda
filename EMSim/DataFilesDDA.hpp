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

#ifndef EMSIM_DATAFILESDDA_HPP_INCLUDED
#define EMSIM_DATAFILESDDA_HPP_INCLUDED

// This are structures representing the DDA-specific data written to the output
// HDF5 files.
// These structures are written using the matlab HDF5 serialization code.

#include <Math/Vector3.hpp>
#include <Math/DiagMatrix3.hpp>
#include <Math/Quaternion.hpp>
#include <Math/Float.hpp>

#include <HDF5/Matlab.hpp>
#include <HDF5/MultiArray.hpp>

#include <EMSim/DataFiles.hpp>

#include <boost/multi_array.hpp>
#include <boost/optional.hpp>

#include <complex>

#include <stdint.h>

namespace EMSim {
  namespace DataFiles {
    // DDA specific parameters
    struct DDAParameters {
      std::string PolarizabilityType;

      ldouble Gamma;

      ldouble Frequency; // in Hz

#define MEMBERS(m)                              \
      m (PolarizabilityType)                    \
      m (Gamma)                                 \
      m (Frequency)
      HDF5_MATLAB_DECLARE_TYPE (DDAParameters, MEMBERS)
#undef MEMBERS
    };

    struct DDADipoleListGeometry {
      std::string Type; // = "DDADipoleList"

      Math::DiagMatrix3<ldouble> GridSpacing; // distance between two dipoles in X/Y/Z-direction, in m (currently value for all directions is the same)
      Math::Vector3<uint32_t> Size; // number of dipoles in the object in X/Y/Z-direction
      Math::Vector3<ldouble> GridOrigin; // the position of grid cell (0,0,0) (in m)
      Math::Quaternion<ldouble> Orientation; // a quaternion describing the rotation transformation being applied to the object after Position has been added

      std::vector<Math::DiagMatrix3<cldouble > > RefractiveIndices; // complex refractive indices for all materials used

      // Can be NULL (i.e. missing in HDF5 file)
      boost::shared_ptr<std::vector<Math::Vector3<uint32_t> > > DipolePositions;
      boost::shared_ptr<std::vector<uint8_t> > DipoleMaterialIndices;

      std::vector<Math::Vector3<ldouble> > Periodicity;

#define MEMBERS(m)                              \
      m (Type)                                  \
      m (GridSpacing)                           \
      m (Size)                                  \
      m (GridOrigin)                            \
      m (Orientation)                           \
      m (RefractiveIndices)                     \
      m (DipolePositions)                       \
      m (DipoleMaterialIndices)                 \
      m (Periodicity)
      HDF5_MATLAB_DECLARE_TYPE (DDADipoleListGeometry, MEMBERS)
#undef MEMBERS
    };

    template <typename ftype>
    struct DDAField {
      std::string FieldName;
      uint32_t BeamPolarization;
      std::vector<Math::Vector3<std::complex<ftype> > > Data;

#define MEMBERS(m)                              \
      m (FieldName)                             \
      m (BeamPolarization)                      \
      m (Data)
      HDF5_MATLAB_DECLARE_TYPE (DDAField, MEMBERS)
#undef MEMBERS
    };

    template <typename ftype>
    struct DDAFieldFile {
      std::string Type; // = "DDAField"

      boost::shared_ptr<DataFiles::Parameters<DDAParameters> > Parameters;
      boost::shared_ptr<DataFiles::DDADipoleListGeometry> Geometry;
      boost::shared_ptr<DataFiles::DDAField<ftype> > Field;

#define MEMBERS(m)                              \
      m (Type)                                  \
      m (Parameters)                            \
      m (Geometry)                              \
      m (Field)
      HDF5_MATLAB_DECLARE_TYPE (DDAFieldFile, MEMBERS)
#undef MEMBERS

      void writeTxt (const Core::OStream& out) const;
      void write (const boost::filesystem::path& basename, boost::optional<std::string> txtExt = boost::none, boost::optional<std::string> hdf5Ext = (std::string) ".hdf5") const;
    };

    struct DDADipoleListGeometryFile {
      std::string Type; // = "Geometry"

      boost::shared_ptr<DataFiles::DDADipoleListGeometry> Geometry;

#define MEMBERS(m)                              \
      m (Type)                                  \
      m (Geometry)
      HDF5_MATLAB_DECLARE_TYPE (DDADipoleListGeometryFile, MEMBERS)
#undef MEMBERS

      void writeTxt (const Core::OStream& out) const;
      void write (const boost::filesystem::path& basename, boost::optional<std::string> txtExt = boost::none, boost::optional<std::string> hdf5Ext = (std::string) ".hdf5") const;
    };

    // Only used for loading
    struct DDADipoleListGeometryLight {
      boost::shared_ptr<Math::DiagMatrix3<ldouble> > GridSpacing;
      boost::shared_ptr<Math::Vector3<ldouble> > GridOrigin;
      boost::shared_ptr<Math::Quaternion<ldouble> > Orientation;
      boost::shared_ptr<std::vector<Math::DiagMatrix3<cldouble > > > RefractiveIndices;
      boost::shared_ptr<std::vector<Math::Vector3<uint32_t> > > DipolePositions;
      boost::shared_ptr<std::vector<uint8_t> > DipoleMaterialIndices;
      boost::shared_ptr<std::vector<Math::Vector3<ldouble> > > Periodicity;
#define MEMBERS(m)                              \
      m (GridSpacing)                           \
      m (GridOrigin)                            \
      m (Orientation)                           \
      m (RefractiveIndices)                     \
      m (DipolePositions)                       \
      m (DipoleMaterialIndices)                 \
      m (Periodicity)
      HDF5_MATLAB_DECLARE_TYPE (DDADipoleListGeometryLight, MEMBERS)
#undef MEMBERS
    };
    struct DDADipoleListGeometryFileLight {
      boost::shared_ptr<DataFiles::DDADipoleListGeometryLight> Geometry;
#define MEMBERS(m)                              \
      m (Geometry)
      HDF5_MATLAB_DECLARE_TYPE (DDADipoleListGeometryFileLight, MEMBERS)
#undef MEMBERS
    };

    // Only used for loading 
    template <typename ftype>
    struct DDAFieldLight {
      std::vector<Math::Vector3<std::complex<ftype> > > Data;
#define MEMBERS(m)                              \
      m (Data)
      HDF5_MATLAB_DECLARE_TYPE (DDAFieldLight, MEMBERS)
#undef MEMBERS
    };
    template <typename ftype>
    struct DDAFieldFileLight {
      boost::shared_ptr<DataFiles::DDAFieldLight<ftype> > Field;
#define MEMBERS(m)                              \
      m (Field)
      HDF5_MATLAB_DECLARE_TYPE (DDAFieldFileLight, MEMBERS)
#undef MEMBERS
    };
  }
}

#endif // !EMSIM_DATAFILESDDA_HPP_INCLUDED
