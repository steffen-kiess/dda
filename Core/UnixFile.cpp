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

#include "UnixFile.hpp"

#if OS_UNIX

#include <Core/Error.hpp>
#include <Core/BoostFilesystem.hpp>

namespace Core {
  UnixFile::Shared::~Shared () {
    if (fd != -1) {
      Core::Error::check ("close", close (fd));
      fd = -1;
    }
  }

  int UnixFile::dup () const {
    return Core::Error::check ("dup", ::dup (fd ()));
  }

  UnixFile UnixFile::open (const boost::filesystem::path& path, int flags, mode_t mode) {
    return UnixFile (Core::Error::check ("open", ::open (path.BOOST_FILE_STRING.c_str (), flags, mode)));
  }
}

#endif // OS_UNIX
