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

#ifndef CORE_FILE_HPP_INCLUDED
#define CORE_FILE_HPP_INCLUDED

// Some file-related utility functions

#include <Core/IStream.forward.hpp>
#include <Core/OStream.forward.hpp>

#include <string>

#include <boost/filesystem/path.hpp>

namespace Core {
  // Return the absolute path of the executable
  boost::filesystem::path getExecutingFile ();
  // Return the absolute path of the directory the executable is in
  boost::filesystem::path getExecutingPath ();

  // Read the file and return it as a std::string
  std::string readFile (const IStream& stream);
  std::string readFile (const boost::filesystem::path& filename);
  // Read from file or from stdin if filename is "-"
  std::string readFileOrStdin (const std::string& filename);

  // Write data to file
  void writeFile (const OStream& stream, const std::string& data);
  void writeFile (const boost::filesystem::path& filename, const std::string& data);
  // Write data to file or to stdout if filename is "-"
  void writeFileOrStdout (const std::string& filename, const std::string& data);
}

#endif // !CORE_FILE_HPP_INCLUDED
