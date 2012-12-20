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

#include "OStream.hpp"

#include <Core/BoostFilesystem.hpp>
#include <Core/StringUtil.hpp>
#include <Core/Error.hpp>

#include <fstream>
#include <iostream>

#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/tee.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

namespace Core {
  namespace {
    class TeeStream;
    class TeeStreamStuff {
      friend class TeeStream;
  
      typedef boost::iostreams::tee_device<std::ostream, std::ostream> DeviceType;
    public:
      typedef boost::iostreams::stream<DeviceType> StreamType;
    private:

      OStream str1;
      OStream str2;
      DeviceType device;
  
      TeeStreamStuff (const OStream& s1, const OStream& s2) :
        str1 (s1), str2 (s2), device (*str1, *str2) {}
    };
    class TeeStream :
      private TeeStreamStuff,
      public TeeStreamStuff::StreamType {
    public:
      TeeStream (const OStream& s1, const OStream& s2) :
        TeeStreamStuff (s1, s2) {
        open (device, 0); // create unbuffered stream
      }
    };
  }

  OStream OStream::tee (const OStream& s1, const OStream& s2) {
    return OStream (new TeeStream (s1, s2));
  }

  OStream OStream::open (const boost::filesystem::path& path, std::ios_base::openmode mode) {
    std::string filename = path.BOOST_FILE_STRING;
    errno = 0;
    OStream ret (new std::ofstream (filename.c_str (), mode));
    if (!ret.good ())
      Core::Error::error ("Core::OStream::open(): " + filename);
    return ret;
  }

  OStream OStream::openNull () {
    return OStream (new boost::iostreams::stream< boost::iostreams::null_sink > ((boost::iostreams::null_sink())));
  }

  OStream OStream::getStdout () {
    return OStream (boost::shared_ptr<std::ostream> (&std::cout, NoopDeallocator ()));
  }
  OStream OStream::getStderr () {
    return OStream (boost::shared_ptr<std::ostream> (&std::cerr, NoopDeallocator ()));
  }

  // Split a C expression on commas which are not inside (), [], '' or ""
  std::vector<std::string> OStream::splitNames (const std::string& str) {
    std::vector<std::string> res;

    std::string cur;
    int level = 0;
    char quoteChar = 0;
    bool escape = false;
    BOOST_FOREACH (char c, str) {
      if (!quoteChar && level == 0 && c == ',') {
        res.push_back (cur);
        cur = "";
        continue;
      }
      cur += c;
      if (escape) {
        escape = false;
        continue;
      }
      if (quoteChar) {
        if (c == '\\')
          escape = true;
        else if (c == quoteChar)
          quoteChar = 0;
        continue;
      }
      if (c == '(' || c == '[')
        level++;
      else if (level > 0 && (c == ']' || c == ')'))
        level--;
      else if (c == '\'' || c == '"')
        quoteChar = c;
    }
    res.push_back (cur);

    BOOST_FOREACH (std::string& s, res)
      boost::trim (s);

    return res;
  }
}
