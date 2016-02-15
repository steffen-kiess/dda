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

#ifndef CORE_ALLOCATOR_HPP_INCLUDED
#define CORE_ALLOCATOR_HPP_INCLUDED

// Allocator class for allocating memory with a particular alignment

#include <Core/Util.hpp>

#include <new>
#include <limits>

#include <boost/static_assert.hpp>

#include <cstdlib>
#include <cerrno>

#if OS_WIN
#ifndef _MSC_VER
#include <mm_malloc.h>
#endif
#endif

namespace Core {
  template <typename T, size_t align = sizeof (void*)> class Allocator {
    BOOST_STATIC_ASSERT (align >= sizeof (void*));
    BOOST_STATIC_ASSERT ((align & (align - 1)) == 0);

  public:
    template <typename U> class rebind {
    public:
      typedef Allocator<U, align> other;
    };

    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef const T* const_pointer;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    pointer address (reference x) const { return &x; }
    const_pointer address (const_reference x) const { return &x; }
  
    pointer allocate (size_type n, UNUSED const void* hint = 0) {
      //return malloc (sizeof (T) * n);
      if (n > max_size ())
        throw std::bad_alloc ();

      //return (pointer) ::operator new (sizeof (T) * n);
      void* p;
#if OS_UNIX
      switch (posix_memalign (&p, align, sizeof (T) * n)) {
#elif 0
      }
#elif OS_WIN
      p = _mm_malloc (sizeof (T) * n, align);
      switch (p ? 0 : errno) {
#else
#error
#endif
      case 0: return (pointer) p;
      case ENOMEM: throw std::bad_alloc ();
      case EINVAL: abort ();
      default: abort ();
      }
    }

    void deallocate (pointer p, UNUSED size_type n) {
      //::operator delete (p);
#if OS_UNIX
      free (p);
#elif OS_WIN
      _mm_free (p);
#else
#error
#endif
    }

    size_type max_size () const throw () {
      return std::numeric_limits<size_type>::max () / sizeof (T);
    }

    void construct (pointer p, const_reference val) {
      new (p) T (val);
    }

    void destroy (pointer p) {
      p->~T ();
    }
  };
}

#endif // !CORE_ALLOCATOR_HPP_INCLUDED
