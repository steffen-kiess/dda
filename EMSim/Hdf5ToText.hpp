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

#ifndef EMSIM_HDF5TOTEXT_HPP_INCLUDED
#define EMSIM_HDF5TOTEXT_HPP_INCLUDED

// Method to convert a HDF5 file to a text file

#include <Core/OStream.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>

namespace EMSim {
  typedef boost::variant<boost::filesystem::path, Core::OStream> StreamOrPathOutput;

  void hdf5ToText (const boost::filesystem::path& input, const StreamOrPathOutput& output, const boost::optional<std::string>& typeOverwrite, bool thetaMax180, bool noPhi);
}

#endif // !EMSIM_HDF5TOTEXT_HPP_INCLUDED
