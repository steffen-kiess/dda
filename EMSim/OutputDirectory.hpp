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

#ifndef EMSIM_OUTPUTDIRECTORY_HPP_INCLUDED
#define EMSIM_OUTPUTDIRECTORY_HPP_INCLUDED

// Code for creating the output directory

#include <Core/Util.hpp>
#include <Core/File.hpp>

#include <string>

#include <boost/filesystem/path.hpp>

namespace EMSim {
  class OutputDirectory {
    NO_COPY_CLASS (OutputDirectory);

    boost::filesystem::path parentDirectory_;
    std::string name_;
    boost::filesystem::path path_;  

  public:
    OutputDirectory (const boost::filesystem::path& parentDirectory);
    ~OutputDirectory ();

    static uint64_t getUniqueId (const boost::filesystem::path& filename);

    const boost::filesystem::path& parentDirectory () const { return parentDirectory_; }
    const std::string& name () const { return name_; }
    const boost::filesystem::path& path () const { return path_; }

    void createTag (const std::string& tag) const;
  };
}

#endif // !EMSIM_OUTPUTDIRECTORY_HPP_INCLUDED
