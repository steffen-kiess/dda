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

#ifndef EMSIM_FORWARD_HPP_INCLUDED
#define EMSIM_FORWARD_HPP_INCLUDED

// Forward declarations of types in the EMSim namespace

namespace EMSim {
  struct CrossSection;

  class Length;

  class AngleList;

  template <class T> class FarFieldEntry;
  template <class T> class JonesMatrix;
  template <class T> class MuellerMatrix;

  namespace DataFiles {
    struct File;
    template <typename MethodType> struct Parameters;
    template <typename ftype> struct JonesFarField;
    template <typename ftype> struct MuellerFarField;
    template <typename ftype, typename MethodType, typename GeometryType> struct JonesFarFieldFile;
    template <typename ftype, typename MethodType, typename GeometryType> struct MuellerFarFieldFile;

    struct DDAParameters;
    struct DDADipoleListGeometry;
  }
}

#endif // !EMSIM_FORWARD_HPP_INCLUDED
