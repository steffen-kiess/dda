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

#include "Matlab.hpp"

#include <Core/CheckedCast.hpp>

#include <unistd.h>

#include <limits>

#include <boost/foreach.hpp>

#include <lib/utf8cpp/utf8.h>

namespace HDF5 {
  MatlabSerializationContext::MatlabSerializationContext (const HDF5::File& file)
    : file_ (file) // , id (0)
  {
  }

  MatlabSerializationContext::~MatlabSerializationContext () {
  }

  MatlabDeserializationContext::MatlabDeserializationContext (const HDF5::File& file)
    : file_ (file)
  {
  }

  MatlabDeserializationContext::~MatlabDeserializationContext () {
  }

  MatlabObject::MatlabObject (const HDF5::Object& o) {
    HDF5::Object object = o;

    bool hasEmptyAttribute;

    for (;;) {
      isOctaveNewFormat_ = object.getType () == H5I_GROUP && object.existsAttribute ("OCTAVE_NEW_FORMAT");
      if (!isOctaveNewFormat_) {
        hasEmptyAttribute = object.existsAttribute ("MATLAB_empty");
        break;
      }

      HDF5::Group grp = (HDF5::Group) object;
      HDF5::DataSet typeDs = (HDF5::DataSet) grp.open ("type");

      ASSERT (typeDs.getSpace ().getSimpleExtentType () == H5S_SCALAR);
      HDF5::DataSpace s = HDF5::DataSpace::create (H5S_SCALAR);

      HDF5::StringType ty = (HDF5::StringType) typeDs.getDataType ();
      ASSERT (!(Exception::check ("H5Tis_variable_str", H5Tis_variable_str (ty.handle ())) != 0));

      /*
        const char* str = NULL;
        HDF5::DataType t = getH5Type<const char*> ();
        typeDs.read (&str, t, s, s);
        std::string str2 (str);
        typeDs.vlenReclaim (&str, t, s);
      */

      size_t size = H5Tget_size (ty.handle ());
      HDF5::Exception::check ("H5Tget_size", size ? 0 : -1);
      std::vector<char> str (size);
      typeDs.read (str.data (), ty, s, s);
      std::string str2 (str.begin (), str.end ());

      size_t pos = str2.find ('\0');
      if (pos != std::string::npos)
        str2 = str2.substr (0, pos);

      if (str2 != "cell") { 
        hasEmptyAttribute = grp.existsAttribute ("OCTAVE_EMPTY_MATRIX");
        object = grp.open ("value");
        octaveType_ = str2;
        break;
      } else {
        HDF5::DataSet dimsDs = (HDF5::DataSet) grp.open ("value/dims");
        HDF5::DataSpace dimsSp = dimsDs.getSpace ();
        ASSERT (dimsSp.getSimpleExtentType () == H5S_SIMPLE);
        ASSERT (dimsSp.getSimpleExtentNdims () == 1);
        hsize_t dim;
        dimsSp.getSimpleExtentDims (&dim);
        ASSERT (dim == 2);
        
        int64_t dims[2];
        dimsDs.read (dims, HDF5::getH5Type<int64_t> (), dimsSp);
        ASSERT (dims[0] == 1);
        ASSERT (dims[1] == 1);

        object = grp.open ("value/_0");
      }
    }

    H5I_type_t type = object.getType ();

    if (type == H5I_DATASET) {
      isStruct_ = false;
      dataSet_ = (HDF5::DataSet) object;
      if (hasEmptyAttribute) {
        isEmpty_ = true;
        isNullDataSpace_ = false;

        HDF5::DataSpace dataSpace = dataSet_.getSpace ();
        ASSERT (dataSpace.getSimpleExtentType () == H5S_SIMPLE);
        ASSERT (dataSpace.getSimpleExtentNdims () == 1);
        hsize_t ndims;
        dataSpace.getSimpleExtentDims (&ndims);
        ASSERT (ndims > 0);

        std::vector<uint64_t> values (Core::checked_cast<size_t> (ndims));
        dataSet_.read (values.data (), getH5Type<uint64_t> (), dataSpace);

        bool foundZero = false;
        for (size_t i = 0; i < ndims; i++)
          if (!values[i])
            foundZero = true;
        ASSERT (foundZero);

        size_.resize (Core::checked_cast<size_t> (ndims));
        for (size_t i = 0; i < ndims; i++) {
          ASSERT (values[i] <= std::numeric_limits<size_t>::max ());
          size_[i] = Core::checked_cast<size_t> (values[i]);
        }
      } else {
        dataSpace_ = dataSet_.getSpace ();
        H5S_class_t extentType = dataSpace_.getSimpleExtentType ();
        //ASSERT (extentType == H5S_SIMPLE || extentType == H5S_NULL);
        ASSERT (extentType == H5S_SIMPLE || extentType == H5S_NULL || extentType == H5S_SCALAR);
        if (extentType == H5S_SIMPLE) {
          isNullDataSpace_ = false;
          size_t dim = dataSpace_.getSimpleExtentNdims ();
          size_.resize (dim);
          std::vector<hsize_t> dims (dim);
          dataSpace_.getSimpleExtentDims (dims.data ());
          isEmpty_ = false;
          for (size_t i = 0; i < dim; i++) {
            ASSERT (dims[i] >= std::numeric_limits<size_t>::min () && dims[i] <= std::numeric_limits<size_t>::max ());
            size_[dim - 1 - i] = Core::checked_cast<size_t> (dims[i]);
            if (!dims[i])
              isEmpty_ = true;
          }
          if (isEmpty_) {
            dataSpace_ = HDF5::DataSpace ();
          }
        } else if (extentType == H5S_SCALAR) {
          isNullDataSpace_ = false;
          isEmpty_ = false;
          //size_.resize (0);
          size_.resize (1);
          size_[0] = 1;
        } else { // H5S_NULL
          isEmpty_ = true;
          isNullDataSpace_ = true;
          size_.resize (1);
          size_[0] = 0;
        }
      }
      if (isEmpty_)
        dataSet_ = HDF5::DataSet ();
    } else if (type == H5I_GROUP) {
      isStruct_ = true;
      group_ = (HDF5::Group) object;
    } else {
      ABORT_MSG ("Unknown object type");
    }
  }

  MatlabObject::~MatlabObject () {
  }

  void MatlabObject::checkScalar () const {
    ASSERT (!isStruct ());
    ASSERT (!isEmpty ());
    size_t ndims = size ().size ();
    if (ndims == 0) {
      return;
    } else if (ndims == 1) {
      ASSERT (size ()[0] == 1);
    } else if (ndims == 2) {
      ASSERT (size ()[0] == 1 && size ()[1] == 1);
    } else {
      ABORT_MSG ("ndims is larger than 2");
    }
  }

  size_t MatlabObject::get1dLength () const {
    size_t ndims = size ().size ();
    if (isEmpty ()) {
      if (!isNullDataSpace ()) {
        ASSERT (ndims <= 2);
        BOOST_FOREACH (size_t s, size ())
          ASSERT (s == 0 || s == 1);
      }
      return 0;
    } else { 
      if (ndims == 1) {
        return size ()[0];
      } else if (ndims == 2) {
        if (size ()[0] == 1)
          return size ()[1];
        else if (size ()[1] == 1)
          return size ()[0];
        else
          ABORT_MSG ("got 2d-dataspace with both dimensions != 1");
      } else {
        ABORT_MSG ("ndims is neither 1 nor 2");
      }
    }
  }

  HDF5::File createMatlabFile (const boost::filesystem::path& filename) {
    FileCreatePropList cprop = FileCreatePropList::create ();
    //cprop.setUserblock (512);
    Exception::check ("H5Pset_userblock", H5Pset_userblock (cprop.handle (), 512));
    HDF5::File file = HDF5::File::open (filename, H5F_ACC_RDWR | H5F_ACC_CREAT | H5F_ACC_TRUNC, cprop);
    char userblock[512];
    memset (userblock, 0, 512);
    snprintf (userblock, 124, "MAT");
    userblock[124] = 0;
    userblock[125] = 2;
    userblock[126] = 'I';
    userblock[127] = 'M';
    int fd = file.getVFDHandleFD ();
#if OS_WIN
    if (lseek (fd, 0, SEEK_SET) < 0) {
      perror ("lseek");
      ABORT ();
    }
    if (write (fd, userblock, 512) < 0) {
      perror ("write");
      ABORT ();
    }
#else
    if (pwrite (fd, userblock, 512, 0) < 0) {
      perror ("pwrite");
      ABORT ();
    }
#endif
    return file;
  }

  void MatlabSerializationContext::add (const SerializationKey& key, const HDF5::Object& obj) {
    ASSERT (obj.isValid ());
    ASSERT (ids.count (key) == 0);
    ids[key] = obj;
  }

  void MatlabSerializationContext::addEmpty (const SerializationKey& key) {
    ASSERT (ids.count (key) == 0);
    ids[key] = HDF5::Object ();
  }

  void MatlabSerializationContextHandle::add (const HDF5::Object& obj) const {
    context ().add (key (), obj);
  }

  void MatlabSerializationContextHandle::addEmpty () const {
    context ().addEmpty (key ());
  }

  HDF5::DataSet MatlabSerializationContextHandle::createDataSet (const HDF5::DataType& data_type, const HDF5::DataSpace& data_space, DataSetCreatePropList dcpl) const {
    // Disable time tracking for objects to make HDF5 files more deterministic
    DataSetCreatePropList dcpl2;
    if (dcpl.isValid ())
      dcpl2 = (DataSetCreatePropList) dcpl.copy ();
    else
      dcpl2 = DataSetCreatePropList::create ();
    HDF5::Exception::check ("H5Pset_obj_track_times", H5Pset_obj_track_times (dcpl2.handle (), false));

    HDF5::DataSet ds = HDF5::DataSet::create (context ().file (), data_type, data_space, dcpl2);
    add (ds);
    return ds;
  }

  HDF5::Group MatlabSerializationContextHandle::createGroup () const {
    // Disable time tracking for objects to make HDF5 files more deterministic
    GroupCreatePropList gcpl = GroupCreatePropList::create ();
    HDF5::Exception::check ("H5Pset_obj_track_times", H5Pset_obj_track_times (gcpl.handle (), false));

    HDF5::Group group = HDF5::Group::create (context ().file (), gcpl);
    writeAttribute (group, "MATLAB_class", "struct");
    add (group);
    return group;
  }

  void MatlabSerializer<bool>::h5MatlabSave (const MatlabSerializationContextHandle& handle, const bool& b) {
    //HDF5::DataSpace dataSpace = HDF5::DataSpace::create (H5S_SCALAR); // Matlab (7.5) cannot read this
    HDF5::DataSpace dataSpace = HDF5::DataSpace::createSimple (1);

    uint8_t data = b ? 1 : 0;
    HDF5::DataType dt = getH5Type<uint8_t> ();
    HDF5::DataSet dataSet = handle.createDataSet (dt, dataSpace);
    writeAttribute (dataSet, "MATLAB_class", "logical");
    writeScalarAttribute<int32_t> (dataSet, "MATLAB_int_decode", 1);
    dataSet.write (&data, dt, dataSpace);
  }
  void MatlabSerializer<bool>::h5MatlabLoad (const MatlabDeserializationContextHandle<bool>& handle) {
    MatlabObject mo (handle.get ());
    uint8_t value = mo.getScalar<uint8_t> ();
    handle.registerValue (boost::make_shared<bool> ((bool) value));
  }

  void MatlabSerializer<std::string>::h5MatlabSave (const MatlabSerializationContextHandle& handle, const std::string& s) {
    std::vector <uint16_t> str;
    utf8::utf8to16 (s.begin (), s.end (), std::back_inserter (str));

    bool useNull = (str.size () == 0) && (H5_VERS_MAJOR < 1 || (H5_VERS_MAJOR == 1 && (H5_VERS_MINOR < 8 || (H5_VERS_MINOR == 8 && H5_VERS_RELEASE < 7))));
    HDF5::DataSpace dataSpace;
    if (useNull)
      dataSpace = HDF5::DataSpace::create (H5S_NULL);
    else
      dataSpace = HDF5::DataSpace::createSimple (str.size (), 1);
    HDF5::DataType dt = getH5Type<uint16_t> ();
    HDF5::DataSet dataSet = handle.createDataSet (dt, dataSpace);
    writeAttribute (dataSet, "MATLAB_class", "char");
    writeScalarAttribute<int32_t> (dataSet, "MATLAB_int_decode", 2);
    if (!useNull)
      // pass in dataSpace as fileSpace to avoid problems when str.data () is NULL (causes "no output buffer" error)
      dataSet.write (str.data (), dt, dataSpace, dataSpace);
  }
  void MatlabSerializer<std::string>::h5MatlabLoad (const MatlabDeserializationContextHandle<std::string>& handle) {
    MatlabObject mo (handle.get ());
    if (mo.isOctaveNewFormat () && (mo.octaveType () == "string" || mo.octaveType () == "sq_string")) {
      std::vector<int16_t> v;
      mo.get1dStdVector (v);
      std::string result (v.begin (), v.end ());
      handle.registerValue (boost::make_shared<std::string> (result));
    } else {
      std::vector<uint16_t> v;
      mo.get1dStdVector (v);
      std::string result;
      utf8::utf16to8 (v.begin (), v.end (), std::back_inserter (result));
      handle.registerValue (boost::make_shared<std::string> (result));
    }
  }
}
