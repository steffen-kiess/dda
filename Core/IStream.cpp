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

#include "IStream.hpp"

#include <Core/BoostFilesystem.hpp>
#include <Core/Error.hpp>

#include <fstream>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/tee.hpp>

#include <iostream>

namespace Core {
  IStream IStream::open (const boost::filesystem::path& path, std::ios_base::openmode mode) {
    std::string filename = path.BOOST_FILE_STRING;
    errno = 0;
    IStream ret (new std::ifstream (filename.c_str (), mode));
    if (!ret.good ())
      Core::Error::error ("Core::IStream::open(): " + filename);
    return ret;
  }

  IStream IStream::getStdin () {
    return IStream (boost::shared_ptr<std::istream> (&std::cin, NoopDeallocator ()));
  }
}
