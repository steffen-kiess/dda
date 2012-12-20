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

#ifndef DDA_DDAPARAMS_HPP_INCLUDED
#define DDA_DDAPARAMS_HPP_INCLUDED

#include <Core/OStream.hpp>

#include <Math/Vector3.hpp>
#include <Math/DiagMatrix3.hpp>
#include <Math/SymMatrix3.hpp>
#include <Math/Float.hpp>

#include <LinAlg/FFTPlan.hpp>

#include <DDA/DipoleGeometry.hpp>
#include <DDA/FPConst.hpp>
#include <DDA/BeamPolarization.hpp>
#include <DDA/Forward.hpp>

#include <cmath>
#include <iomanip>
#include <sstream>
#include <complex>
#include <algorithm>

#include <boost/shared_ptr.hpp>

namespace DDA {
  // Class containing all the parameters needed for creating the dmatrix and
  // setting up the iterative solver without anything depending on the incident
  // field
  template <class T>
  class DDAParams {
#ifdef SWIG_VISIBILITY_WORKAROUND
  public:
#endif
    typedef T ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

  private:
    static const uint32_t vecStrideAlign = 16;

    boost::shared_ptr<const Geometry> geometry_;
    std::string geometryString_;
    boost::shared_ptr<const DipoleGeometry> dipoleGeometry_;
    ftype lambda_;
    ftype gamma_;
    cuint32_t procs_;

    ftype gridUnit_;
    Math::Vector3<ftype> periodicity1_;
    Math::Vector3<ftype> periodicity2_;

    cuint32_t nvCount_;
    cuint32_t vecStride_;
    cuint32_t vecSize_;
    Math::Vector3<cuint32_t> gridSize_;
    ftype waveNum_;
    ftype kd_;

    std::vector<uint32_t> localCount_;
    std::vector<uint32_t> localNvCount_;
    std::vector<uint32_t> localVecSize_;
    std::vector<uint32_t> localVecStride_;
    std::vector<uint32_t> localGridX_;
    std::vector<uint32_t> localBoxZ_;
    std::vector<uint32_t> localX0_;
    std::vector<uint32_t> localZ0_;
    std::vector<uint32_t> localVec0_;
    cuint32_t localGridXMax_;
    std::vector<uint32_t> localBlockOffset_;

    static csize_t roundUp (csize_t value, csize_t d) {
      return ((value + (d - 1)) / d) * d;
    }

  public:
    DDAParams (const boost::shared_ptr<const Geometry>& geometry,
               const std::string& geometryString,
               const boost::shared_ptr<const DipoleGeometry>& dipoleGeometry,
               ftype lambda,
               bool supportNonPot,
               cuint32_t procs = 1,
               Math::Vector3<cuint32_t> gridSize = Math::Vector3<cuint32_t> (0, 0, 0),
               ftype gamma = 0.001,
               const std::vector<uint32_t>& localGridX = std::vector<uint32_t> (0),
               const std::vector<uint32_t>& localBoxZ = std::vector<uint32_t> (0));

    const Geometry& geometry () const { return *geometry_; }
    template <typename U> bool geometryIs () const { return dynamic_cast<const U*> (geometry_.get ()); }
    template <typename U> const U& geometryAs () const { return dynamic_cast<const U&> (geometry ()); }
    const std::string& geometryString () const { return geometryString_; }
    std::string& geometryString () { return geometryString_; }
    const DipoleGeometry& dipoleGeometry () const { return *dipoleGeometry_; }
    ftype gridUnit () const { return gridUnit_; }
    ftype lambda () const { return lambda_; }
    Math::Vector3<ftype> periodicity1 () const { return periodicity1_; }
    Math::Vector3<ftype> periodicity2 () const { return periodicity2_; }
    ftype gamma () const { return gamma_; }

    ftype frequency () const { return Const::speed_of_light / lambda (); }

    const Math::Vector3<cuint32_t> gridSize () const { return gridSize_; }
    ftype gridUnitVol () const { return static_cast<ftype> (dipoleGeometry ().gridUnitVol ()); }
    ftype waveNum () const { return waveNum_; }
    ftype kd () const { return kd_; }
    int periodicityDimension () const { return dipoleGeometry ().periodicityDimension (); }

    cuint32_t procs () const { return procs_; }

    cuint32_t cnvCount () const { return nvCount_; }
    cuint32_t cvecStride () const { return vecStride_; }
    cuint32_t cvecSize () const { return vecSize_; }
    cuint32_t cgridX () const { return gridSize ().x (); }
    cuint32_t cgridY () const { return gridSize ().y (); }
    cuint32_t cgridZ () const { return gridSize ().z (); }

    uint32_t nvCount () const { return cnvCount () (); }
    uint32_t vecStride () const { return cvecStride () (); }
    uint32_t vecSize () const { return cvecSize () (); }
    uint32_t gridX () const { return cgridX () (); }
    uint32_t gridY () const { return cgridY () (); }
    uint32_t gridZ () const { return cgridZ () (); }

    cuint32_t localCCount (cuint32_t i) const { ASSERT (i < procs ()); return localCount_[i ()]; }
    cuint32_t localCNvCount (cuint32_t i) const { ASSERT (i < procs ()); return localNvCount_[i ()]; }
    cuint32_t localCVecSize (cuint32_t i) const { ASSERT (i < procs ()); return localVecSize_[i ()]; }
    cuint32_t localCVecStride (cuint32_t i) const { ASSERT (i < procs ()); return localVecStride_[i ()]; }
    cuint32_t localCGridX (cuint32_t i) const { ASSERT (i < procs ()); return localGridX_[i ()]; }
    cuint32_t localCBoxZ (cuint32_t i) const { ASSERT (i < procs ()); return localBoxZ_[i ()]; }
    cuint32_t localCX0 (cuint32_t i) const { ASSERT (i < procs ()); return localX0_[i ()]; }
    cuint32_t localCZ0 (cuint32_t i) const { ASSERT (i < procs ()); return localZ0_[i ()]; }
    cuint32_t localCVec0 (cuint32_t i) const { ASSERT (i < procs ()); return localVec0_[i ()]; }
    cuint32_t localCGridXMax () const { return localGridXMax_; }
    cuint32_t localCBlockOffset (cuint32_t proc, cuint32_t block) const {
      ASSERT (proc < procs ());
      ASSERT (block <= procs ());
      return localBlockOffset_[(block * procs () + proc) ()];
    }
    cuint32_t localCBlocksSize (cuint32_t proc) const { return localCBlockOffset (proc, procs ()); }

    uint32_t localCount (cuint32_t i) const { return localCCount (i) (); }
    uint32_t localNvCount (cuint32_t i) const { return localCNvCount (i) (); }
    uint32_t localVecSize (cuint32_t i) const { return localCVecSize (i) (); }
    uint32_t localVecStride (cuint32_t i) const { return localCVecStride (i) (); }
    uint32_t localGridX (cuint32_t i) const { return localCGridX (i) (); }
    uint32_t localBoxZ (cuint32_t i) const { return localCBoxZ (i) (); }
    uint32_t localX0 (cuint32_t i) const { return localCX0 (i) (); }
    uint32_t localZ0 (cuint32_t i) const { return localCZ0 (i) (); }
    uint32_t localVec0 (cuint32_t i) const { return localCVec0 (i) (); }
    uint32_t localGridXMax () const { return localCGridXMax () (); }
    uint32_t localBlockOffset (cuint32_t proc, cuint32_t block) const { return localCBlockOffset (proc, block) (); }
    uint32_t localBlocksSize (cuint32_t proc) const { return localCBlocksSize (proc) (); }

    template <typename U>
    Math::Vector3<U> get (const std::vector<U>& vector, size_t index) const {
      return Math::Vector3<U> (vector[index], vector[index + vecStride ()], vector[index + 2 * vecStride ()]);
    }

    template <typename U>
    void set (std::vector<U>& vector, size_t index, const Math::Vector3<U>& value) const {
      vector[index] = value.x ();
      vector[index + vecStride ()] = value.y ();
      vector[index + 2 * vecStride ()] = value.z ();
    }

    void multMat (const std::vector<Math::DiagMatrix3<ctype> >& matrix, const std::vector<ctype>& vector, std::vector<ctype>& result) const {
      ASSERT (matrix.size () == dipoleGeometry ().materials ().size ());
      ASSERT (vector.size () == vecSize ());
      ASSERT (result.size () == vecSize ());

      for (size_t i = 0; i < cnvCount (); i++)
        set (result, i, matrix[dipoleGeometry ().getMaterialIndex (i)] * get (vector, i));
    }

    boost::shared_ptr<std::vector<ctype> > multMat (const std::vector<Math::DiagMatrix3<ctype> >& matrix, const std::vector<ctype>& vector) const {
      boost::shared_ptr<std::vector<ctype> > result = boost::make_shared<std::vector<ctype> > (vecSize ());
      multMat (matrix, vector, *result);
      return result;
    }

    void multMatInv (const std::vector<Math::DiagMatrix3<ctype> >& matrix, const std::vector<ctype>& vector, std::vector<ctype>& result) const {
      ASSERT (matrix.size () == dipoleGeometry ().materials ().size ());
      ASSERT (vector.size () == vecSize ());
      ASSERT (result.size () == vecSize ());

      for (size_t i = 0; i < cnvCount (); i++)
        set (result, i, matrix[dipoleGeometry ().getMaterialIndex (i)].inverse () * get (vector, i));
    }

    std::string toString (const Beam<ftype>& beam, const CoupleConstants<ftype>& cc1, const CoupleConstants<ftype>& cc2) const;

    void dump (const Beam<ftype>& beam, const CoupleConstants<ftype>& cc1, const CoupleConstants<ftype>& cc2, const Core::OStream& out = Core::OStream::getStdout ()) const {
      out << toString (beam, cc1, cc2);
    }
  };

  // This class is separate from DDAParams because for LDR and LDR+avgpol
  // polarizability the couple constants depend on the propagation direction
  // and for LDR also on the incident polarization
  template <class T>
  class CoupleConstants {
#ifdef SWIG_VISIBILITY_WORKAROUND
  public:
#endif
    typedef T ftype;
    typedef std::complex<ftype> ctype;
    typedef FPConst<ftype> Const;

  private:
    std::vector<Math::DiagMatrix3<ctype> > cc_;
    std::vector<Math::DiagMatrix3<ctype> > cc_sqrt_;
    std::vector<Math::DiagMatrix3<ctype> > chi_inv_;

  public:
    CoupleConstants (const DDAParams<ftype>& ddaParams, const Beam<ftype>& beam, BeamPolarization pol, const PolarizabilityDescription<ftype>& polDesc);

    const std::vector<Math::DiagMatrix3<ctype> >& cc () const { return cc_; }
    const std::vector<Math::DiagMatrix3<ctype> >& cc_sqrt () const { return cc_sqrt_; }
    const std::vector<Math::DiagMatrix3<ctype> >& chi_inv () const { return chi_inv_; }
  };
}

#endif // !DDA_DDAPARAMS_HPP_INCLUDED
