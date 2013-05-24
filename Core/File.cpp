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

#include "File.hpp"

#include <Core/Util.hpp>
#include <Core/Assert.hpp>
#include <Core/CheckedIntegerAlias.hpp>
#include <Core/Error.hpp>
#include <Core/WindowsError.hpp>
#include <Core/IStream.hpp>
#include <Core/OStream.hpp>

#include <vector>

#if OS_WIN
#include <windows.h>
#endif

namespace Core {
  boost::filesystem::path getExecutingFile () {
#if OS_UNIX
    std::vector<char> retbuf (2);
    ssize_t ret;
    while ((ret = readlink ("/proc/self/exe", retbuf.data (), retbuf.size ())) == (ssize_t) retbuf.size ())
      retbuf.resize ((csize_t (retbuf.size ()) * 2) ());
    Core::Error::check ("readlink", ret);
    ASSERT ((size_t) ret <= retbuf.size ());
    return std::string (retbuf.data (), ret);
#elif OS_WIN
    std::vector<char> retbuf (2);
    ssize_t ret;
    while ((ret = GetModuleFileName (NULL, retbuf.data (), retbuf.size ())) == (ssize_t) retbuf.size ())
      retbuf.resize ((csize_t (retbuf.size ()) * 2) ());
    Core::WindowsError::check ("GetModuleFileName", ret);
    ASSERT ((size_t) ret <= retbuf.size ());
    return std::string (retbuf.data (), ret);
#else
#error Unknown OS
#endif
  }

  boost::filesystem::path getExecutingPath () {
    return getExecutingFile ().parent_path ();
  }

  std::string readFile (const IStream& stream) {
    std::string data;
    while (!stream->eof ()) {
      char c;
      stream->get (c);
      ASSERT (!stream->bad ());
      if (!stream->eof ())
        data += c;
    }
    return data;
  }

  std::string readFile (const boost::filesystem::path& filename) {
    return readFile (Core::IStream::open (filename));
  }

  std::string readFileOrStdin (const std::string& filename) {
    if (filename == "-")
      return readFile (Core::IStream::getStdin ());
    else
      return readFile (filename);
  }

  void writeFile (const OStream& stream, const std::string& data) {
    stream->write (data.c_str (), data.length ());
    stream.assertGood ();
  }

  void writeFile (const boost::filesystem::path& filename, const std::string& data) {
    writeFile (Core::OStream::open (filename), data);
  }

  void writeFileOrStdout (const std::string& filename, const std::string& data) {
    if (filename == "-")
      writeFile (Core::OStream::getStdout (), data);
    else
      writeFile (filename, data);
  }

  class SystemCommandException : public Exception {
    int status_;
    const std::string command_;

  public:
    SystemCommandException (int status, const std::string& command) : status_ (status), command_ (command) {
    }
    virtual ~SystemCommandException () throw () {
    }

    int status () const { return status_; }
    const std::string& command () const { return command_; }

    virtual std::string message () const {
      return Core::sprintf ("Execution of command '%s' returned value %d", command (), status ());
    }
  };

  void system (const std::string& cmd) {
    int status = Core::Error::check ("system", ::system (cmd.c_str ()));
    if (status != 0)
      throw SystemCommandException (status, cmd);
  }
}
