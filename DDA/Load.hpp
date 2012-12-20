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

#ifndef DDA_LOAD_HPP_INCLUDED
#define DDA_LOAD_HPP_INCLUDED

// Various methods for loading geometries / fields

#include <Core/IStream.hpp>
#include <Core/Util.hpp>

#include <Math/FPTemplateInstances.hpp>

#include <DDA/Forward.hpp>

#include <boost/filesystem/path.hpp>

#include <string>
#include <complex>

namespace DDA {
  void loadDipoleGeometry (DipoleGeometry& dipoleGeometry, const Core::IStream& infile);
  void loadDipoleGeometry (DipoleGeometry& dipoleGeometry, const boost::filesystem::path& filename);

  template <typename ftype>
  class Load {
    STATIC_CLASS (Load);

  public:
    static void loadFields (const Core::IStream& in, const DDAParams<ftype>& ddaParams, const std::string& name, std::vector<std::complex<ftype> >& field);
    static void loadFields (const boost::filesystem::path& inputFile, const DDAParams<ftype>& ddaParams, const std::string& name, std::vector<std::complex<ftype> >& field);
    static void loadDipPol (const boost::filesystem::path& inputBasename, const DDAParams<ftype>& ddaParams, std::vector<std::complex<ftype> >& field);
  };

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, Load)
}

#endif // !DDA_LOAD_HPP_INCLUDED
