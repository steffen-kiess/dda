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

#ifndef HDF5_SERIALIZATION_HPP_INCLUDED
#define HDF5_SERIALIZATION_HPP_INCLUDED

#include "Serialization.forward.hpp"

// Code for HDF5 serialization. Unlike the code in Matlab.hpp the resulting
// files cannot be read by matlab. Not really tested / used currently.

#include <Core/Assert.hpp>
#include <HDF5/Type.hpp>
#include <HDF5/File.hpp>
#include <HDF5/Group.hpp>
#include <HDF5/DataSet.hpp>
#include <HDF5/SerializationKey.hpp>

#include <map>
#include <set>
#include <sstream>
#include <vector>

#include <stdint.h>

#include <boost/filesystem/path.hpp>
#include <boost/any.hpp>

namespace HDF5 {
  template <typename T> struct Serializer;
  template <typename T> struct Serializer {
    static inline void h5Save (SerializationContext& context, const T& t) {
      t.h5Save (context);
    }
    static inline void h5Load (DeserializationContext& context, ObjectReference name, HDF5::DataSet& dataSet) {
      T::h5Load (context, name, dataSet);
    }
  };

  class SerializationContext {
    const HDF5::File file;
    HDF5::Group refGroup;
    std::set<SerializationKey> inProgress;
    std::map<SerializationKey, ObjectReference> references;
    uint64_t id;

    class InProgressInserter {
      std::set<SerializationKey>& inProgress;
      SerializationKey value;

    public:
      InProgressInserter (std::set<SerializationKey>& inProgress, SerializationKey value) : inProgress (inProgress), value (value) {
        ASSERT (inProgress.count (value) == 0);
        inProgress.insert (value);
      }

      ~InProgressInserter () {
        ASSERT (inProgress.count (value) == 1);
        inProgress.erase (value);
      }
    };

    const char* groupName() {
      return "#refs#";
    }
    
    const HDF5::Group& getRefGroup () {
      if (!refGroup.isValid ()) {
        if (!file.rootGroup ().exists (groupName ())) {
          HDF5::Group group = HDF5::Group::create (file);
          file.rootGroup ().link (groupName (), group);
          refGroup = group;
        } else {
          refGroup = (Group) file.rootGroup ().open (groupName ());
        }
      }
      return refGroup;
    }

  public:
    SerializationContext (const HDF5::File& file);
    ~SerializationContext ();

    void add (const SerializationKey& key, const ObjectReference& ref) {
      ASSERT (references.count (key) == 0);
      references[key] = ref;
    }

    std::string newName () {
      ASSERT (id + 1 != 0);
      uint64_t myId = ++id;
      std::stringstream str;
      str << "object-" << myId;
      return str.str ();
    }

    template <typename T>
    HDF5::DataSet createDataSet (const T& ref, const HDF5::DataType& data_type, const HDF5::DataSpace& data_space = HDF5::DataSpace::create (H5S_SCALAR), const DataSetCreatePropList& dcpl = DataSetCreatePropList ()) {
      std::string name = newName ();
      ASSERT (references.count (SerializationKey::create (ref)) == 0);
      HDF5::DataSet ds = HDF5::DataSet::create (file, data_type, data_space, dcpl);
      references[SerializationKey::create (ref)] = ds.reference ();
      getRefGroup ().link (name, ds);
      return ds;
    }

    template <typename T>
    ObjectReference write (const std::vector<T>& vector) {
      HDF5::DataType type = *getH5Type<T> ();
      size_t count = vector.size ();
      HDF5::DataSpace space = HDF5::DataSpace::createSimple (count);
      std::string name = newName ();
      HDF5::DataSet dataset = HDF5::DataSet::create (file, type, space);
      getRefGroup ().link (name, dataset);
      dataset.write (vector.data (), type);
      return dataset.reference ();
    }

    template <typename T>
    ObjectReference get (const T& object) {
      SerializationKey addr = SerializationKey::create (object);
      if (references.count (addr)) {
        return references[addr];
      } else {
        {
          InProgressInserter ipi (inProgress, addr);
          Serializer<T>::h5Save (*this, object);
        }
        ASSERT (references.count (addr));
        return references[addr];
      }
    }
  };

  class DeserializationContext {
    const HDF5::File file;
    std::set<ObjectReference> inProgress;
    std::map<ObjectReference, boost::any> objects;

  public:
    DeserializationContext (const HDF5::File& file);
    ~DeserializationContext ();

    template <typename T>
    void registerValue (ObjectReference name, boost::shared_ptr<T> ptr) {
      ASSERT (!objects.count (name));
      ASSERT (inProgress.count (name));
      objects[name] = boost::any (ptr);
      inProgress.erase (name);
    }

    template <typename T>
    boost::shared_ptr<T> resolve (ObjectReference name) {
      if (objects.count (name))
        return boost::any_cast<boost::shared_ptr<T> > (objects[name]);
      ASSERT (!inProgress.count (name));
      inProgress.insert (name);
      HDF5::DataSet dataSet = (DataSet) name.dereference (file);
      Serializer<T>::h5Load (*this, name, dataSet);
      ASSERT (!inProgress.count (name));
      return boost::any_cast<boost::shared_ptr<T> > (objects[name]);
    }
  };

  template <typename T> inline void serialize (const boost::filesystem::path& outputFile, const std::string& name, const T& object) {
    ASSERT (name != "");
    HDF5::File file = HDF5::File::open (outputFile, H5F_ACC_RDWR | H5F_ACC_CREAT | H5F_ACC_TRUNC);
    SerializationContext context (file);
    file.rootGroup ().link (name, context.get (object).dereference (file));
  }

  template <typename T> inline boost::shared_ptr<T> deserialize (const boost::filesystem::path& outputFile, const std::string& name) {
    ASSERT (name != "");
    HDF5::File file = HDF5::File::open (outputFile, H5F_ACC_RDONLY);
    DeserializationContext context (file);
    return context.resolve<T> (file.rootGroup ().open (name).reference ());
  }

  inline bool isHdf5 (const boost::filesystem::path& file) {
    return HDF5::File::isHDF5 (file);
  }
}

#endif // !HDF5_SERIALIZATION_HPP_INCLUDED
