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

#include "OutputDirectory.hpp"

#include <Core/BoostFilesystem.hpp>
#include <Core/Error.hpp>
#include <Core/CheckedIntegerAlias.hpp>
#include <Core/IStream.hpp>
#include <Core/OStream.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>

#ifdef BOOST_USE_WINDOWS_H
// Workaround for boost bug, needed when BOOST_USE_WINDOWS_H ist set
#undef BOOST_USE_WINDOWS_H
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#define BOOST_USE_WINDOWS_H
#else
#include <boost/interprocess/sync/file_lock.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#endif

namespace EMSim {
  uint64_t OutputDirectory::getUniqueId (const boost::filesystem::path& filename) {
    std::string fileCounter = filename.BOOST_FILE_STRING;
    std::string fileNew = filename.BOOST_FILE_STRING + ".new";
    std::string fileLock = filename.BOOST_FILE_STRING + ".lck";

    // Create lock file if it does not exist
    Core::OStream::open (fileLock, std::ios_base::in | std::ios_base::out | std::ios_base::app);

    // Acquire lock
    boost::interprocess::file_lock lock (fileLock.c_str ());
    boost::interprocess::scoped_lock<boost::interprocess::file_lock> sc (lock);

    // Create counter file if it does not exist
    Core::OStream::open (fileCounter, std::ios_base::in | std::ios_base::out | std::ios_base::app);

    // Read old value
    cuint64_t value = 0;
    {
      Core::IStream file = Core::IStream::open (fileCounter);
      uint64_t val;
      *file >> val;
      ASSERT (!file->bad ());
      if (file->eof ()) {
        value = 0;
      } else {
        ASSERT (file->good ());
        value = val;
      }
    }

    cuint64_t newValue = value + 1;

    // Write new value
    {
      Core::OStream file = Core::OStream::open (fileNew);
      file << newValue << std::endl;
      file->flush ();
      file.assertGood ();
    }

#if OS_WIN
    if (remove (fileCounter.c_str ()) < 0 && errno != ENOENT) {
      perror ("remove");
      ABORT ();
    }
#endif
    // Rename file
    if (rename (fileNew.c_str (), fileCounter.c_str ()) < 0) {
      perror ("rename");
      ABORT ();
    }

    // release lock on return
    return value ();
  }

  OutputDirectory::OutputDirectory (const boost::filesystem::path& parentDirectory) {
    parentDirectory_ = parentDirectory;
    boost::filesystem::create_directories (parentDirectory);

    cuint64_t id = getUniqueId (parentDirectory / "counter");
    name_ = "output_" + boost::lexical_cast<std::string> (id);

    path_ = parentDirectory / name_;
    ASSERT (!boost::filesystem::exists (path_));
    boost::filesystem::create_directory (path_);
  }
  OutputDirectory::~OutputDirectory () {
  }

  void OutputDirectory::createTag (UNUSED const std::string& tag) const {
    ASSERT (tag != "");
    ASSERT (tag.find ('/') == std::string::npos);
    ASSERT (tag[0] != '.');
    ASSERT (tag.substr (0, 7) != "output_");

#if OS_UNIX
    boost::filesystem::path link = parentDirectory () / tag;
    Core::Error::checkIgnore ("unlink", unlink (link.BOOST_FILE_STRING.c_str ()), ENOENT);
    Core::Error::check ("symlink", symlink (name ().c_str (), link.BOOST_FILE_STRING.c_str ()));
#endif
  }
}
