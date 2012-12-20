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

#ifndef OPENCL_POINTER_HPP_INCLUDED
#define OPENCL_POINTER_HPP_INCLUDED

// A pointer to a memory location on an opencl device
//
// This is implemented as a pair of a cl_mem and a offset.
// The first template parameter is the type of the object being pointed to
// (can be const).
// The second parameter is a boolean. If the second parameter is true the
// pointer will be a strong reference (i.e. it will prevent the cl_mem object
// from being freed), if it is false the pointer will be weak (and if the
// pointer is used after the cl_mem object has been freed this will cause
// undefined behavior.
//
// Support normal pointer arithmetic.
// To get a pointer to a member of a struct use
// OpenCL::Pointer<MemberType> ptr2 = ptr1 + &Struct::member;

#include <Core/CheckedIntegerAlias.hpp>
#include <Core/Type.hpp>

#include <OpenCL/Bindings.hpp>

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/type_traits/is_union.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/alignment_of.hpp>
#include <boost/type_traits/aligned_storage.hpp>
#include <boost/static_assert.hpp>

#include <ostream>

#include <cstddef>

namespace OpenCL {
  struct PointerNull_t {};
  static const PointerNull_t PointerNull = PointerNull_t ();

  template <typename T, bool strong = false> class Pointer {
    struct TraitsWeak {
      typedef cl_mem type;
      static cl_mem getClMem (const type& value) {
        return value;
      }
      static cl_mem create (const cl::Buffer& buf) {
        return buf ();
      }
      static void init (type& obj, cl_mem mem) {
        obj = mem;
      }
    };
    struct TraitsStrong {
      typedef cl::Buffer type;
      static cl_mem getClMem (const type& value) {
        return value ();
      }
      static const type& create (const cl::Buffer& buf) {
        return buf;
      }
      static void init (type& obj, cl_mem mem) {
        ASSERT (obj () == NULL);
        if (mem)
          cl::detail::errHandler (clRetainMemObject (mem), "clRetainMemObject");
        obj () = mem;
      }
    };
    typedef typename boost::mpl::if_c<strong, TraitsStrong, TraitsWeak>::type Traits;

    typedef typename boost::remove_const<T>::type MutableT;

    typename Traits::type buffer_;
    cptrdiff_t offset_;

    struct InvisibleType { cl_mem mem () { return 0; } cptrdiff_t offset () { return 0; } };
    struct InvisibleType2 { cl_mem mem () { return 0; } cptrdiff_t offset () { return 0; } };
    struct InvisibleType3 { cl_mem mem () { return 0; } cptrdiff_t offset () { return 0; } };

  public:
    cl_mem mem () const {
      return Traits::getClMem (buffer_);
    }
    cptrdiff_t offset () const {
      return offset_;
    }

    Pointer () : offset_ (0) {
      Traits::init (buffer_, NULL);
    }

    Pointer (UNUSED PointerNull_t x) : offset_ (0) {
      Traits::init (buffer_, NULL);
    }

    Pointer (const cl::Buffer& buf, cptrdiff_t offset) : buffer_ (Traits::create (buf)), offset_ (offset) {
      ASSERT (this->mem () != NULL || offset == 0);
      ASSERT (offset >= 0);
    }
    Pointer (cl_mem mem, cptrdiff_t offset) : offset_ (offset) {
      Traits::init (buffer_, mem);
      ASSERT (this->mem () != NULL || offset == 0);
      ASSERT (offset >= 0);
    }

    // Convert nonconst -> const
    Pointer (typename boost::mpl::if_<boost::is_const<T>, const Pointer<typename boost::remove_const<T>::type, strong>&, InvisibleType2>::type ptr) : offset_ (ptr.offset ()) {
      Traits::init (buffer_, ptr.mem ());
    }
    // Convert nonconst, strong -> const, weak
    Pointer (typename boost::mpl::if_c<boost::is_const<T>::value && !strong, const Pointer<typename boost::remove_const<T>::type, true>&, InvisibleType3>::type ptr) : offset_ (ptr.offset ()) {
      Traits::init (buffer_, ptr.mem ());
    }

    // Convert strong -> weak
    Pointer (typename boost::mpl::if_c<!strong, const Pointer<T, true>&, InvisibleType>::type ptr) : offset_ (ptr.offset ()) {
      Traits::init (buffer_, ptr.mem ());
    }
    // Convert weak -> strong
    explicit Pointer (typename boost::mpl::if_c<strong, const Pointer<T, false>&, InvisibleType>::type ptr) : offset_ (ptr.offset ()) {
      Traits::init (buffer_, ptr.mem ());
    }

    struct UnspecifiedBoolMember {};
    class UnspecifiedBoolStruct {
      friend class Pointer;
      UnspecifiedBoolMember member;
    };
    typedef UnspecifiedBoolMember UnspecifiedBoolStruct::* UnspecifiedBoolType;
    operator UnspecifiedBoolType() const {
      return (mem () == NULL) ? 0 : &UnspecifiedBoolStruct::member;
    }

    void read (const cl::CommandQueue& queue, MutableT& value) const {
      cl::detail::errHandler (clEnqueueReadBuffer (queue (), mem (), true, offset () (), sizeof (T), &value, 0, NULL, NULL), "clEnqueueReadBuffer");
    }

    const T read (const cl::CommandQueue& queue) const {
      MutableT value;
      read (queue, value);
      return value;
    }

    void write (const cl::CommandQueue& queue, const T& value) const {
      BOOST_STATIC_ASSERT (!boost::is_const<T>::value);
      cl::detail::errHandler (clEnqueueWriteBuffer (queue (), mem (), true, offset () (), sizeof (T), &value, 0, NULL, NULL), "clEnqueueWriteBuffer");
    }
  };

  template <typename U, typename T, bool strong>
  inline Pointer<U> operator+ (const Pointer<T, strong>& g, U T::* ptr) {
    BOOST_STATIC_ASSERT (!boost::is_const<T>::value);
    ASSERT (g);
    ASSERT (ptr != 0);
    Pointer<U> ret (g.mem (), g.offset () + Core::Type::getOffset (ptr));
    return ret;
  }
  template <typename U, typename T, bool strong>
  inline Pointer<const U> operator+ (const Pointer<const T, strong>& g, U T::* ptr) {
    BOOST_STATIC_ASSERT (!boost::is_const<T>::value);
    ASSERT (g);
    ASSERT (ptr != 0);
    Pointer<const U> ret (g.mem (), g.offset () + Core::Type::getOffset (ptr));
    return ret;
  }

  template <typename T, bool strong>
  inline Pointer<T> operator+ (const Pointer<T, strong>& g, cptrdiff_t diff) {
    BOOST_STATIC_ASSERT (!boost::is_const<T>::value);
    ASSERT (g || diff == 0);
    Pointer<T> ret (g.mem (), g.offset () + diff * sizeof (T));
    return ret;
  }
  template <typename T, bool strong>
  inline Pointer<const T> operator+ (const Pointer<const T, strong>& g, cptrdiff_t diff) {
    BOOST_STATIC_ASSERT (!boost::is_const<T>::value);
    ASSERT (g || diff == 0);
    Pointer<const T> ret (g.mem (), g.offset () + diff * sizeof (T));
    return ret;
  }

  template <typename T, bool strong>
  inline Pointer<T> operator- (const Pointer<T, strong>& g, cptrdiff_t diff) {
    BOOST_STATIC_ASSERT (!boost::is_const<T>::value);
    ASSERT (g || diff == 0);
    Pointer<T> ret (g.mem (), g.offset () - diff * sizeof (T));
    return ret;
  }
  template <typename T, bool strong>
  inline Pointer<const T> operator- (const Pointer<const T, strong>& g, cptrdiff_t diff) {
    BOOST_STATIC_ASSERT (!boost::is_const<T>::value);
    ASSERT (g || diff == 0);
    Pointer<const T> ret (g.mem (), g.offset () - diff * sizeof (T));
    return ret;
  }

  template <typename T, bool strong>
  inline std::ostream& operator<< (std::ostream& stream, const Pointer<T, strong>& ptr) {
    stream << "Pointer<";
    stream << Core::Type::getName<T> ();
    if (strong)
      stream << ", true";
    stream << ">";
    stream << " (" << ptr.mem () << ", " << ptr.offset () << ")";
    return stream;
  }
}

#endif // !OPENCL_POINTER_HPP_INCLUDED
