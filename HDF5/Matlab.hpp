/*
 * Copyright (c) 2010-2013 Steffen Kieß
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

#ifndef HDF5_MATLAB_HPP_INCLUDED
#define HDF5_MATLAB_HPP_INCLUDED

// Code for matlab-compatible HDF5 serialization

#include <Core/Util.hpp>
#include <Core/Assert.hpp>

#include <HDF5/BaseTypes.hpp>
#include <HDF5/Type.hpp>
#include <HDF5/Util.hpp>
#include <HDF5/File.hpp>
#include <HDF5/Group.hpp>
#include <HDF5/DataSet.hpp>
#include <HDF5/DataSpace.hpp>
#include <HDF5/DataType.hpp>
#include <HDF5/SerializationKey.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/any.hpp>
#include <boost/optional.hpp>
#include <boost/utility/in_place_factory.hpp>

#include <set>
#include <map>
#include <vector>

// From boost=1.46.1-5ubuntu3 /usr/include/boost/thread/locks.hpp
#define MY_BOOST_DEFINE_HAS_MEMBER_CALLED(member_name)                  \
  template<typename T, bool=boost::is_class<T>::value>                  \
  struct has_member_called_##member_name                                \
  {                                                                     \
    BOOST_STATIC_CONSTANT(bool, value=false);                           \
  };                                                                    \
                                                                        \
  template<typename T>                                                  \
  struct has_member_called_##member_name<T,true>                        \
  {                                                                     \
    typedef char true_type;                                             \
    struct false_type                                                   \
    {                                                                   \
      true_type dummy[2];                                               \
    };                                                                  \
                                                                        \
    struct fallback { int member_name; };                               \
    struct derived:                                                     \
      T, fallback                                                       \
      {                                                                 \
        derived();                                                      \
      };                                                                \
                                                                        \
    template<int fallback::*> struct tester;                            \
                                                                        \
    template<typename U>                                                \
      static false_type has_member(tester<&U::member_name>*);           \
    template<typename U>                                                \
      static true_type has_member(...);                                 \
                                                                        \
    BOOST_STATIC_CONSTANT(                                              \
                          bool, value=sizeof(has_member<derived>(0))==sizeof(true_type)); \
  }

namespace HDF5 {
  class MatlabSerializationContext;
  class MatlabDeserializationContext;

  class MatlabSerializationContextHandle {
    MatlabSerializationContext& context_;
    SerializationKey key_;

  public:
    MatlabSerializationContextHandle (MatlabSerializationContext& context, const SerializationKey& key) : context_ (context), key_ (key) {}

    MatlabSerializationContext& context () const {
      return context_;
    }

    const SerializationKey& key () const {
      return key_;
    }

    void add (const HDF5::Object& obj) const;
    void addEmpty () const;

    HDF5::DataSet createDataSet (const HDF5::DataType& data_type, const HDF5::DataSpace& data_space = HDF5::DataSpace (), DataSetCreatePropList dcpl = DataSetCreatePropList ()) const;

    HDF5::Group createGroup () const;
  };

  template <typename T>
  class MatlabDeserializationContextHandle {
    MatlabDeserializationContext& context_;
    ObjectReference key_;

  public:
    MatlabDeserializationContextHandle (MatlabDeserializationContext& context, ObjectReference key) : context_ (context), key_ (key) {}

    MatlabDeserializationContext& context () const {
      return context_;
    }

    const ObjectReference& key () const {
      return key_;
    }

    void registerValue (const boost::shared_ptr<T>& ptr) const;

    Object get () const;
  };

  template <typename T>
  class MatlabDeserializationContextHandleDirect {
    MatlabDeserializationContext& context_;
    ObjectReference key_;
    T& ref_;

  public:
    MatlabDeserializationContextHandleDirect (MatlabDeserializationContext& context, ObjectReference key, T& ref) : context_ (context), key_ (key), ref_ (ref) {}

    MatlabDeserializationContext& context () const {
      return context_;
    }

    const ObjectReference& key () const {
      return key_;
    }

    Object get () const;

    T& ref () const {
      return ref_;
    }
  };

  template <typename T> struct MatlabTypeImpl {};

  template <typename T> inline HDF5::DataType getMatlabH5MemoryType () {
    return MatlabTypeImpl<T>::getMemory ();
  }
  template <typename T> inline HDF5::DataType getMatlabH5FileType () {
    return MatlabTypeImpl<T>::getFile ();
  }

  template <typename T> struct MatlabSerializer;

  namespace Intern {
    MY_BOOST_DEFINE_HAS_MEMBER_CALLED (h5MatlabSave);
    MY_BOOST_DEFINE_HAS_MEMBER_CALLED (h5MatlabLoad);
    MY_BOOST_DEFINE_HAS_MEMBER_CALLED (h5MatlabLoadNull);
    MY_BOOST_DEFINE_HAS_MEMBER_CALLED (h5MatlabLoadDirect);
    MY_BOOST_DEFINE_HAS_MEMBER_CALLED (h5MatlabLoadNullDirect);

    template <typename T, bool hasMatlabLoadSave> struct MatlabSerializerIntern;

    template <typename T> struct MatlabSerializerIntern<T, true> {
      static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const T& t) {
        return t.h5MatlabSave (handle);
      }

      static inline void h5MatlabSaveToGroup (MatlabSerializationContext& context, const HDF5::Group& group, const T& t) {
        return t.h5MatlabSaveToGroup (context, group);
      }

      static inline void h5MatlabLoad (const MatlabDeserializationContextHandle<T>& handle) {
        T::h5MatlabLoad (handle);
      }

      static inline void h5MatlabLoadDirect (const MatlabDeserializationContextHandleDirect<T>& handle) {
        T::h5MatlabLoadDirect (handle);
      }
    };

    template <typename T, bool hasMatlabLoadCanHandleNull> struct MatlabSerializerIntern2;

    template <typename T, bool hasMatlabLoadDirect> struct MatlabSerializerIntern3;

    template <typename T, bool hasMatlabLoad> struct MatlabSerializerIntern4;
    template <typename T> struct MatlabSerializerIntern4<T, false> {
      static inline void h5MatlabLoad (const MatlabDeserializationContextHandle<T>& handle) {
        boost::shared_ptr<T> ptr = boost::make_shared<T> ();
        handle.registerValue (ptr);
        MatlabSerializer<T>::h5MatlabLoadDirect (MatlabDeserializationContextHandleDirect<T> (handle.context (), handle.key (), *ptr));
      }
    };
    template <typename T> struct MatlabSerializerIntern4<T, true> {
      static inline void h5MatlabLoad (const MatlabDeserializationContextHandle<T>& handle) {
        MatlabSerializer<T>::h5MatlabLoad (handle);
      }
    };
  }

  template <typename T> struct MatlabSerializer {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const T& t) {
      Intern::MatlabSerializerIntern<T, Intern::has_member_called_h5MatlabSave<T>::value>::h5MatlabSave (handle, t);
    }

    static inline void h5MatlabSaveToGroup (MatlabSerializationContext& context, const HDF5::Group& group, const T& t) {
      Intern::MatlabSerializerIntern<T, Intern::has_member_called_h5MatlabSave<T>::value>::h5MatlabSaveToGroup (context, group, t);
    }

    static inline void h5MatlabLoad (const MatlabDeserializationContextHandle<T>& handle) {
      Intern::MatlabSerializerIntern<T, Intern::has_member_called_h5MatlabLoad<T>::value>::h5MatlabLoad (handle);
    }
  };

  class MatlabSerializationContext {
    const HDF5::File file_;
    std::set<SerializationKey> inProgress;
    std::map<SerializationKey, HDF5::Object> ids;

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

    /* Needed for serializing cell arrays etc.
    HDF5::Group refGroup;
    uint64_t id;

    const char* groupName() {
      return "#refs#";
    }
    
    const HDF5::Group& getRefGroup () {
      if (!refGroup.isValid ()) {
        if (!file ().rootGroup ().exists (groupName ())) {
          HDF5::Group group = HDF5::Group::create (file ());
          file ().rootGroup ().link (groupName (), group);
          refGroup = group;
        } else {
          refGroup = (Group) file ().rootGroup ().open (groupName ());
        }
      }
      return refGroup;
    }

    std::string newName () {
      ASSERT (id + 1 != 0);
      uint64_t myId = ++id;
      std::stringstream str;
      str << "object-" << myId;
      return str.str ();
    }
    */

  public:
    MatlabSerializationContext (const HDF5::File& file);
    ~MatlabSerializationContext ();

    const HDF5::File& file () const {
      return file_;
    }

    void add (const SerializationKey& key, const HDF5::Object& obj);
    void addEmpty (const SerializationKey& key);

    template <typename T>
    HDF5::Object get (const T& object) {
      SerializationKey key = SerializationKey::create (object);
      if (ids.count (key)) {
        return ids[key];
      } else {
        {
          InProgressInserter ipi (inProgress, key);
          MatlabSerializer<T>::h5MatlabSave (MatlabSerializationContextHandle (*this, key), object);
        }
        ASSERT (ids.count (key));
        return ids[key];
      }
    }

    template <typename T>
    void writeToGroup (const HDF5::Group& group, const T& object) {
      SerializationKey key = SerializationKey::create (object);
      add (key, group);
      ASSERT (inProgress.count (key) == 0);
      MatlabSerializer<T>::h5MatlabSaveToGroup (*this, group, object);
    }
  };

  class MatlabDeserializationContext {
    const HDF5::File file_;
    std::set<DeserializationKey> inProgress;
    std::set<DeserializationKey> inProgress2;
    std::map<DeserializationKey, boost::any> objects;

  public:
    MatlabDeserializationContext (const HDF5::File& file);
    ~MatlabDeserializationContext ();

    const HDF5::File& file () const {
      return file_;
    }

    template <typename T>
    void registerValue (ObjectReference name, boost::shared_ptr<T> ptr) {
      DeserializationKey key = DeserializationKey::create<T> (name);
      ASSERT (!objects.count (key));
      ASSERT (inProgress.count (key));
      ASSERT (inProgress2.count (key));
      objects[key] = boost::any (ptr);
      inProgress.erase (key);
    }

    template <typename T>
    boost::shared_ptr<T> resolve (ObjectReference name) {
      if (name.isNull ())
        return Intern::MatlabSerializerIntern2<T, Intern::has_member_called_h5MatlabLoadNull<MatlabSerializer<T> >::value>::h5MatlabLoadNull (*this);
      DeserializationKey key = DeserializationKey::create<T> (name);
      if (objects.count (key))
        return boost::any_cast<boost::shared_ptr<T> > (objects[key]);
      ASSERT (!inProgress.count (key));
      ASSERT (!inProgress2.count (key));
      inProgress.insert (key);
      inProgress2.insert (key);
      //MatlabSerializer<T>::h5MatlabLoad (MatlabDeserializationContextHandle<T> (*this, name));
      Intern::MatlabSerializerIntern4<T, Intern::has_member_called_h5MatlabLoad<MatlabSerializer<T> >::value>::h5MatlabLoad (MatlabDeserializationContextHandle<T> (*this, name));
      ASSERT (!inProgress.count (key));
      ASSERT (inProgress2.count (key));
      inProgress2.erase (key);
      return boost::any_cast<boost::shared_ptr<T> > (objects[key]);
    }

    template <typename T>
    boost::shared_ptr<T> resolve (const HDF5::Object& obj) {
      ASSERT (obj.file () == file ());
      return resolve<T> (obj.reference ());
    }

    template <typename T>
    const T& resolveValue (ObjectReference name, const char* memberName = NULL) {
      DeserializationKey key = DeserializationKey::create<T> (name);
      boost::shared_ptr<T> ptr = resolve<T> (name);
      ASSERT_MSG (ptr, "Got null reference / missing value during deserialization" + (memberName ? std::string (" for member name `") + memberName + "'" : ""));
      ASSERT (!inProgress2.count (key));
      return *ptr;
    } 

    template <typename T>
    const T& resolveValue (const HDF5::Object& obj) {
      ASSERT (obj.file () == file ());
      return resolveValue<T> (obj.reference ());
    }

    template <typename T>
    void resolveValueDirect (T& ref, ObjectReference name, const char* memberName = NULL) {
      Intern::MatlabSerializerIntern3<T, Intern::has_member_called_h5MatlabLoadDirect<MatlabSerializer<T> >::value>::h5MatlabLoadDirect (MatlabDeserializationContextHandleDirect<T> (*this, name, ref), memberName);
    } 

    template <typename T>
    void resolveValueDirect (T& ref, const HDF5::Object& obj) {
      ASSERT (obj.file () == file ());
      resolveValueDirect<T> (ref, obj.reference ());
    }
  };

  template <typename T>
  inline void MatlabDeserializationContextHandle<T>::registerValue (const boost::shared_ptr<T>& ptr) const {
    context ().registerValue (key (), ptr);
  }

  template <typename T>
  inline Object MatlabDeserializationContextHandle<T>::get () const {
    return key ().dereference (context ().file ());
  }

  template <typename T>
  inline Object MatlabDeserializationContextHandleDirect<T>::get () const {
    return key ().dereference (context ().file ());
  }

  class MatlabObject {
    bool isStruct_;

    // isStruct_ = true
    HDF5::Group group_;

    // isStruct_ = false
    bool isEmpty_;
    bool isNullDataSpace_;

    // isStruct_ = false, isNullDataSpace_ = false
    std::vector<size_t> size_;

    // isStruct_ = false, isEmpty_ = false
    HDF5::DataSet dataSet_;
    HDF5::DataSpace dataSpace_;

    bool isOctaveNewFormat_;
    // isOctaveNewFormat_ = true
    std::string octaveType_;

  public:
    MatlabObject (const HDF5::Object& object);
    ~MatlabObject ();

    bool isStruct () const { return isStruct_; }

    const HDF5::Group& group () const {
      ASSERT (isStruct ());
      return group_;
    }

    bool isEmpty () const {
      ASSERT (!isStruct ());
      return isEmpty_;
    }
    bool isNullDataSpace () const {
      ASSERT (!isStruct ());
      return isNullDataSpace_;
    }

    const std::vector<size_t>& size () const {
      ASSERT (!isNullDataSpace ());
      return size_;
    }

    const HDF5::DataSet& dataSet () const {
      ASSERT (!isEmpty ());
      return dataSet_;
    }
    const HDF5::DataSpace& dataSpace () const {
      ASSERT (!isEmpty ());
      return dataSpace_;
    }

    bool isOctaveNewFormat () const {
      return isOctaveNewFormat_;
    }
    const std::string& octaveType () const {
      ASSERT (isOctaveNewFormat ());
      return octaveType_;
    }

    void checkScalar () const;
    template <typename T>
    inline void getScalar (T& value) {
      checkScalar ();
      HDF5::DataType type = getMatlabH5MemoryType<T> ();
      dataSet ().read (&value, type, dataSpace ());
    }
    template <typename T>
    inline T getScalar () {
      T value;
      getScalar (value);
      return value;
    }

    size_t get1dLength () const;
    template <typename T>
    inline void get1dStdVector (std::vector<T>& vector) {
      size_t len = get1dLength ();
      vector.resize (len);
      HDF5::DataType type = getMatlabH5MemoryType<T> ();
      if (len)
        dataSet ().read (vector.data (), type, dataSpace ());
    }
    template <typename T>
    inline void get1dValues (T* data, size_t num) {
      size_t len = get1dLength ();
      ASSERT (num == len);
      HDF5::DataType type = getMatlabH5MemoryType<T> ();
      if (len)
        dataSet ().read (data, type, dataSpace ());
    }
  };

  HDF5::File createMatlabFile (const boost::filesystem::path& outputFile);

  template <typename T> inline HDF5::Object matlabSerializeGetObject (const HDF5::File& file, const T& object) {
    MatlabSerializationContext context (file);
    return context.get (object);
  }
  template <typename T> inline void matlabSerialize (const HDF5::File& file, const std::string& name, const T& object) {
    ASSERT (name != "");
    HDF5::Object obj = matlabSerializeGetObject (file, object);
    file.rootGroup ().link (name, obj);
  }
  template <typename T> inline void matlabSerialize (const boost::filesystem::path& outputFile, const std::string& name, const T& object) {
    // TODO: create .new file + rename?
    HDF5::File file = createMatlabFile (outputFile);
    matlabSerialize (file, name, object);
  }

  template <typename T> inline void matlabSerialize (const HDF5::Group& group, const T& object) {
    MatlabSerializationContext context (group.file ());
    context.writeToGroup (group, object);
  }
  template <typename T> inline void matlabSerialize (const HDF5::File& file, const T& object) {
    matlabSerialize (file.rootGroup (), object);
  }
  template <typename T> inline void matlabSerialize (const boost::filesystem::path& outputFile, const T& object) {
    // TODO: create .new file + rename?
    HDF5::File file = createMatlabFile (outputFile);
    matlabSerialize (file, object);
  }

  template <typename T> inline boost::shared_ptr<T> matlabDeserialize (const HDF5::File& file, const std::string& name = ".") {
    ASSERT (name != "");
    MatlabDeserializationContext context (file);
    return context.resolve<T> (file.rootGroup ().open (name).reference ());
  }
  template <typename T> inline boost::shared_ptr<T> matlabDeserialize (const boost::filesystem::path& outputFile, const std::string& name = ".") {
    HDF5::File file = HDF5::File::open (outputFile, H5F_ACC_RDONLY);
    return matlabDeserialize<T> (file, name);
  }

  template <typename T> struct MatlabSerializer<boost::shared_ptr<const T> > {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const boost::shared_ptr<const T>& v) {
      if (v)
        MatlabSerializer<T>::h5MatlabSave (handle, *v);
      else
        handle.addEmpty ();
    }
  };
  template <typename T> struct MatlabSerializer<boost::shared_ptr<T> > {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const boost::shared_ptr<const T>& v) {
      if (v)
        MatlabSerializer<T>::h5MatlabSave (handle, *v);
      else
        handle.addEmpty ();
    }
  };

  namespace Intern {
    template <typename T> struct MatlabSerializerIntern<T, false> {
      static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const T& t) {
        //HDF5::DataSpace dataSpace = HDF5::DataSpace::create (H5S_SCALAR); // Matlab (7.5) cannot read this
        HDF5::DataSpace dataSpace = HDF5::DataSpace::createSimple (1);

        HDF5::DataType memType = getMatlabH5MemoryType<T> ();
        HDF5::DataType fileType = getMatlabH5FileType<T> ();
        HDF5::DataSet dataSet = handle.createDataSet (fileType, dataSpace);
        writeAttribute (dataSet, "MATLAB_class", MatlabTypeImpl<T>::matlabClass ());
        dataSet.write (&t, memType, dataSpace);
      }
      // TODO: avoid this overload (and use the MatlabSerializerIntern4::h5MatlabLoad() method)?
      static inline void h5MatlabLoad (const MatlabDeserializationContextHandle<T>& handle) {
        MatlabObject mo (handle.get ());
        boost::shared_ptr<T> ptr = boost::make_shared<T> ();
        handle.registerValue (ptr);
        mo.getScalar (*ptr);
      }
      static inline void h5MatlabLoadDirect (const MatlabDeserializationContextHandleDirect<T>& handle) {
        MatlabObject mo (handle.get ());
        mo.getScalar (handle.ref ());
      }
    };

    template <typename T, bool hasMatlabLoadNullDirect> struct MatlabSerializerIntern5;
    template <typename T> struct MatlabSerializerIntern5<T, false> {
      static inline void h5MatlabLoadNullDirect (UNUSED MatlabDeserializationContext& context, UNUSED T& ref, const char* memberName) {
        ABORT_MSG ("Got null reference / missing value during deserialization" + (memberName ? std::string (" for member name `") + memberName + "'" : ""));
      }
      static inline boost::shared_ptr<T> h5MatlabLoadNull (UNUSED MatlabDeserializationContext& context) {
        return boost::shared_ptr<T> ();
      }
    };
    template <typename T> struct MatlabSerializerIntern5<T, true> {
      static inline void h5MatlabLoadNullDirect (MatlabDeserializationContext& context, T& ref, UNUSED const char* memberName) {
        MatlabSerializer<T>::h5MatlabLoadNullDirect (context, ref);
      }
      static inline boost::shared_ptr<T> h5MatlabLoadNull (MatlabDeserializationContext& context) {
        boost::shared_ptr<T> ptr = boost::make_shared<T> ();
        MatlabSerializer<T>::h5MatlabLoadNullDirect (context, *ptr);
        return ptr;
      }
    };

    template <typename T> struct MatlabSerializerIntern2<T, false> {
      static inline boost::shared_ptr<T> h5MatlabLoadNull (MatlabDeserializationContext& context) {
        //return boost::shared_ptr<T> ();
        return Intern::MatlabSerializerIntern5<T, Intern::has_member_called_h5MatlabLoadNullDirect<MatlabSerializer<T> >::value>::h5MatlabLoadNull (context);
      }
    };
    template <typename T> struct MatlabSerializerIntern2<T, true> {
      static inline boost::shared_ptr<T> h5MatlabLoadNull (MatlabDeserializationContext& context) {
        return MatlabSerializer<T>::h5MatlabLoadNull (context);
      }
    };

    template <typename T> struct MatlabSerializerIntern3<T, false> {
      static inline void h5MatlabLoadDirect (const MatlabDeserializationContextHandleDirect<T>& handle, const char* memberName) {
        handle.ref () = handle.context ().template resolveValue<T> (handle.key (), memberName);
      }
    };
    template <typename T> struct MatlabSerializerIntern3<T, true> {
      static inline void h5MatlabLoadDirect (const MatlabDeserializationContextHandleDirect<T>& handle, const char* memberName) {
        if (handle.key ().isNull ())
          Intern::MatlabSerializerIntern5<T, Intern::has_member_called_h5MatlabLoadNullDirect<MatlabSerializer<T> >::value>::h5MatlabLoadNullDirect (handle.context (), handle.ref (), memberName);
        else
          MatlabSerializer<T>::h5MatlabLoadDirect (handle);
      }
    };
  }

  template <typename T> struct MatlabSerializer<std::vector<T> > {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const std::vector<T>& v) {
      bool useNull = (v.size () == 0) && (H5_VERS_MAJOR < 1 || (H5_VERS_MAJOR == 1 && (H5_VERS_MINOR < 8 || (H5_VERS_MINOR == 8 && H5_VERS_RELEASE < 7))));
      HDF5::DataSpace dataSpace;
      if (useNull)
        dataSpace = HDF5::DataSpace::create (H5S_NULL);
      else
        dataSpace = HDF5::DataSpace::createSimple (v.size ());
      HDF5::DataType memType = getMatlabH5MemoryType<T> ();
      HDF5::DataType fileType = getMatlabH5FileType<T> ();
      HDF5::DataSet dataSet = handle.createDataSet (fileType, dataSpace);
      writeAttribute (dataSet, "MATLAB_class", MatlabTypeImpl<T>::matlabClass ());
      if (!useNull)
        // pass in dataSpace as fileSpace to avoid problems when v.data () is NULL (causes "no output buffer" error)
        dataSet.write (v.data (), memType, dataSpace, dataSpace);
    }
    static inline void h5MatlabLoadDirect (const MatlabDeserializationContextHandleDirect<std::vector<T> >& handle) {
      MatlabObject mo (handle.get ());
      mo.get1dStdVector (handle.ref ());
    }
  };

  template <> struct MatlabSerializer<bool> {
    static void h5MatlabSave (const MatlabSerializationContextHandle& handle, const bool& b);
    static void h5MatlabLoad (const MatlabDeserializationContextHandle<bool>& handle);
  };

  template <> struct MatlabSerializer<std::string> {
    static void h5MatlabSave (const MatlabSerializationContextHandle& handle, const std::string& s);
    static void h5MatlabLoad (const MatlabDeserializationContextHandle<std::string>& handle);
  };

  template <> struct MatlabSerializer<const char*> {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const char*& s) {
      MatlabSerializer<std::string>::h5MatlabSave (handle, s);
    }
  };
  template <> struct MatlabSerializer<char*> {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const char*& s) {
      MatlabSerializer<std::string>::h5MatlabSave (handle, s);
    }
  };
  template <size_t size> struct MatlabSerializer<char[size]> {
    typedef char T[size];
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const T& s) {
      MatlabSerializer<std::string>::h5MatlabSave (handle, s);
    }
  };

  template <> struct MatlabSerializer<HDF5::DataSet> {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const HDF5::DataSet& ds) {
      ASSERT (ds.isValid ());
      handle.add (ds);
    }
  };

  template <typename T> struct MatlabSerializer<boost::optional<T> > {
    static inline void h5MatlabSave (const MatlabSerializationContextHandle& handle, const boost::optional<T>& value) {
      if (value)
        MatlabSerializer<T>::h5MatlabSave (handle, *value);
      else
        handle.addEmpty ();
    }
    static inline void h5MatlabLoadNullDirect (UNUSED MatlabDeserializationContext& context, boost::optional<T>& ref) {
      //return boost::make_shared<boost::optional<T> > ();
      ref = boost::none;
    }
    static inline void h5MatlabLoadDirect (const MatlabDeserializationContextHandleDirect<boost::optional<T> >& handle) {
      ASSERT (!handle.key ().isNull ());

      //boost::shared_ptr<boost::optional<T> > ptr = boost::make_shared<boost::optional<T> > ();
      //handle.registerValue (ptr);
      //*ptr = handle.context ().template resolveValue<T> (handle.key ());

      //boost::shared_ptr<boost::optional<T> > ptr = boost::make_shared<boost::optional<T> > (T ());
      //handle.registerValue (ptr);
      //handle.context ().template resolveValueDirect<T> (**ptr, handle.key (), NULL);

      handle.ref () = boost::in_place ();
      handle.context ().template resolveValueDirect<T> (*handle.ref (), handle.key (), NULL);
    }
  };

#define DEF2(ctype, filetype, matlab)           \
  template <> struct MatlabTypeImpl<ctype> {    \
    static HDF5::DataType getMemory () {        \
      return getH5Type<ctype> ();               \
    }                                           \
    static HDF5::DataType getFile () {          \
      return getH5Type<filetype> ();            \
    }                                           \
    static std::string matlabClass () {         \
      return matlab;                            \
    }                                           \
  }
#define DEFC2(ctype, filetype, matlab)                          \
  DEF2(ctype, filetype, matlab);                                \
  template <> struct MatlabTypeImpl<std::complex<ctype> > {     \
    static HDF5::DataType getMemory () {                        \
      return getH5Type<std::complex<ctype> > ();                \
    }                                                           \
    static HDF5::DataType getFile () {                          \
      return getH5Type<std::complex<filetype> > ();             \
    }                                                           \
    static std::string matlabClass () {                         \
      return matlab;                                            \
    }                                                           \
  }
#define DEFC(ctype, matlab) DEFC2 (ctype, ctype, matlab)
  DEFC (float, "single");
  DEFC (double, "double");
  DEFC2 (long double, double, "double");
  DEFC (uint8_t, "uint8");
  DEFC (uint16_t, "uint16");
  DEFC (uint32_t, "uint32");
  DEFC (uint64_t, "uint64");
  DEFC (int8_t, "int8");
  DEFC (int16_t, "int16");
  DEFC (int32_t, "int32");
  DEFC (int64_t, "int64");
#undef DEFC
#undef DEFC2
#undef DEF2

  namespace Intern {
    template <typename T>
    inline void loadToVar (T& ref, MatlabDeserializationContext& context, ObjectReference name, const char* memberName) {
      //ref = context.resolveValue<T> (name, memberName);
      context.resolveValueDirect<T> (ref, name, memberName);
    }

    template <typename T>
    inline void loadToVar (boost::shared_ptr<T>& ref, MatlabDeserializationContext& context, ObjectReference name, UNUSED const char* memberName) {
      ref = context.resolve<T> (name);
    }
  }
}

#define HDF5_MATLAB_ADD_MEMBER_NAME(member, name)                       \
  group.linkIfNotNull (name, context.get (this->member));
#define HDF5_MATLAB_ADD_MEMBER(member) HDF5_MATLAB_ADD_MEMBER_NAME (member, #member)
#define HDF5_MATLAB_TYPE(name)                                          \
  private:                                                              \
  typedef name HDF5_Matlab_CurrentType;                                 \
public:                                                                 \
 inline void h5MatlabSave (const ::HDF5::MatlabSerializationContextHandle& handle) const { \
   ::HDF5::Group group = handle.createGroup ();                           \
   h5MatlabSaveToGroup (handle.context (), group);                      \
 }                                                                      \
 inline void h5MatlabSaveToGroup (::HDF5::MatlabSerializationContext& context, const HDF5::Group& group) const {
#define HDF5_MATLAB_TYPE_END                    \
  }

#define HDF5_MATLAB2_ADD_MEMBER_NAME(member, name)                      \
  /*::Core::OStream::getStderr () << "Loading " << (name) << std::endl;*/ \
  ::HDF5::Intern::loadToVar (value->member, handle.context (), mo.group ().getReferenceIfExists (name), name);
#define HDF5_MATLAB2_ADD_MEMBER(member) HDF5_MATLAB2_ADD_MEMBER_NAME (member, #member)
#define HDF5_MATLAB2_TYPE(name)                                         \
  public:                                                               \
  static inline void h5MatlabLoad (const ::HDF5::MatlabDeserializationContextHandle<HDF5_Matlab_CurrentType>& handle) { \
  ::HDF5::MatlabObject mo (handle.get ());                              \
  ASSERT (mo.isStruct ());                                              \
  ::boost::shared_ptr<HDF5_Matlab_CurrentType> value = ::boost::make_shared<HDF5_Matlab_CurrentType> (); \
  handle.registerValue (value);
#define HDF5_MATLAB2_TYPE_END                   \
  }

#define HDF5_MATLAB_DECLARE_TYPE(name, memberMacros)    \
  HDF5_MATLAB_TYPE (name)                               \
  memberMacros (HDF5_MATLAB_ADD_MEMBER)                 \
  HDF5_MATLAB_TYPE_END                                  \
  HDF5_MATLAB2_TYPE (name)                              \
  memberMacros (HDF5_MATLAB2_ADD_MEMBER)                \
  HDF5_MATLAB2_TYPE_END

#endif // !HDF5_MATLAB_HPP_INCLUDED
