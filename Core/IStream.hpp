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

#ifndef CORE_ISTREAM_HPP_INCLUDED
#define CORE_ISTREAM_HPP_INCLUDED

#include "IStream.forward.hpp"

// Core::IStream is a reference-counted input stream
// Unlike std::istream most operations check the stream state and throw an
// exception in case of an error.

#include <Core/Assert.hpp>
#include <Core/Util.hpp>
#include <Core/StringUtil.hpp>

#include <istream>

#include <boost/smart_ptr.hpp>
#include <boost/filesystem/path.hpp>

namespace Core {
  class IStream {
    boost::shared_ptr<std::istream> stream;

    struct NoopDeallocator {
      void operator() (UNUSED std::istream* str) {
      }
    };

    IStream (boost::shared_ptr<std::istream> stream) : stream (stream) {}

  public:
    // IStream will call delete on str when the reference count drops to zero
    IStream (std::istream* str) : stream (str) {
      ASSERT (str);
    }

    bool good () const {
      //ASSERT (stream);
      return stream->good ();
    }
    void assertGood () const {
      ASSERT (good ());
    }

    std::istream& operator* () const {
      //ASSERT (stream);
      return *stream;
    }
    std::istream* operator-> () const {
      //ASSERT (stream);
      return stream.get ();
    }

    template <typename T>
    const IStream& operator >> (T& t) const {
      assertGood ();
      **this >> t;
      assertGood ();
      return *this;
    }
    // for i/o manipulators
    const IStream& operator >> (std::istream& (*t)(std::istream&)) const {
      assertGood ();
      **this >> t;
      assertGood ();
      return *this;
    }

    // str will not be destructed
    static IStream get (std::istream& str) {
      return IStream (boost::shared_ptr<std::istream> (&str, NoopDeallocator ()));
    }
    static IStream getStdin ();

    static IStream open (const boost::filesystem::path& path, std::ios_base::openmode mode = std::ios_base::in);
  };
}

#endif // !CORE_ISTREAM_HPP_INCLUDED
