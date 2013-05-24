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

#include "DataFilesDDA.hpp"

#include <Core/OStream.hpp>

#include <Math/FPTemplateInstances.hpp>

#include <boost/lexical_cast.hpp>

namespace EMSim {
  namespace DataFiles {
    template <typename ftype>
    void DDAFieldFile<ftype>::writeTxt (const Core::OStream& out) const {
      std::string name = Field->FieldName;
      if (Field->BeamPolarization != 0)
        name += "-Pol" + boost::lexical_cast<std::string> (Field->BeamPolarization);

      out << "#x y z |" << name << "|^2 " << name << ".x.r " << name << ".x.i " << name << ".y.r " << name << ".y.i " << name << ".z.r " << name << ".z.i\n";
      out->precision (10);
      Math::Vector3<ldouble> origin = Geometry->GridOrigin;
      Math::DiagMatrix3<ldouble> spacing = Geometry->GridSpacing;
      ASSERT (Geometry->DipolePositions);
      for (uint32_t i = 0; i < Field->Data.size (); i++) {
        Math::Vector3<std::complex<ftype> > vec = Field->Data[i];
        Math::Vector3<ldouble> coord = origin + spacing * (*Geometry->DipolePositions)[i];
        ftype normalized = Math::abs2 (vec);
    
        out << coord.x () << " " << coord.y () << " " << coord.z () << " "
            << normalized << " "
            << vec.x ().real () << " " << vec.x ().imag () << " " << vec.y ().real () << " " << vec.y ().imag () << " " << vec.z ().real () << " " << vec.z ().imag ()
            << "\n";
      }
    }

    template <typename ftype>
    void DDAFieldFile<ftype>::write (const boost::filesystem::path& basename, boost::optional<std::string> txtExt, boost::optional<std::string> hdf5Ext) const {
      if (hdf5Ext)
        HDF5::matlabSerialize (basename.parent_path () / (basename.BOOST_FILENAME_STRING + *hdf5Ext), *this);

      if (txtExt)
        writeTxt (Core::OStream::open (basename.parent_path () / (basename.BOOST_FILENAME_STRING + *txtExt)));
    }

    void DDADipoleListGeometryFile::writeTxt (const Core::OStream& out) const {
      ASSERT (Geometry);
      ASSERT (Geometry->DipolePositions);
      ASSERT (Geometry->DipoleMaterialIndices);

      size_t count = Geometry->DipolePositions->size ();
      ASSERT (Geometry->DipoleMaterialIndices->size () == count);


      //const char* endl = "\r\n";
      const char* endl = "\n";
      out << "#box size: " << Geometry->Size.x () << "x" << Geometry->Size.y () << "x" << Geometry->Size.z () << endl;
      uint32_t matCount = 0;
      for (uint32_t i = 0; i < count; i++)
        matCount = std::max (matCount, static_cast<uint32_t> ((*Geometry->DipoleMaterialIndices)[i]) + 1);
      if (matCount != 1)
        out << "Nmat=" << matCount << endl;
      for (uint32_t i = 0; i < count; i++) {
        Math::Vector3<uint32_t> coords = (*Geometry->DipolePositions)[i];
        out << coords.x () << " " << coords.y () << " " << coords.z ();
        if (matCount != 1)
          out << " " << static_cast<uint32_t> ((*Geometry->DipoleMaterialIndices)[i]) + 1;
        out << endl;
        //out << coords.x () + 1 << " " << coords.y () + 1 << " " << coords.z () + 1 << " " << static_cast<uint32_t> ((*Geometry->DipoleMaterialIndices)[]) + 1 << endl;
      }
    }

    void DDADipoleListGeometryFile::write (const boost::filesystem::path& basename, boost::optional<std::string> txtExt, boost::optional<std::string> hdf5Ext) const {
      if (hdf5Ext)
        HDF5::matlabSerialize (basename.parent_path () / (basename.BOOST_FILENAME_STRING + *hdf5Ext), *this);

      if (txtExt)
        writeTxt (Core::OStream::open (basename.parent_path () / (basename.BOOST_FILENAME_STRING + *txtExt)));
    }

    CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE_S, DDAFieldFile)
  }
}
