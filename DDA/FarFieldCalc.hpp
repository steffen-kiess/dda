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

#ifndef DDA_FARFIELDCALC_HPP_INCLUDED
#define DDA_FARFIELDCALC_HPP_INCLUDED

// Code for calculating the far field and storing the result

#include <Core/OStream.hpp>
#include <Core/BoostFilesystem.hpp>

#include <Math/FPTemplateInstances.hpp>

#include <EMSim/AngleList.hpp>
#include <EMSim/MuellerCalculus.hpp>
#include <EMSim/JonesCalculus.hpp>
#include <EMSim/DataFiles.hpp>

#include <DDA/Forward.hpp>

#include <HDF5/Matlab.hpp>

#include <complex>

#include <boost/multi_array.hpp>

namespace DDA {
  class FarFieldOption {
    std::string outputName_;
    EMSim::GridAngleList angleList_;
    bool storeJones_;
    bool storeMueller_;
    bool noPhi_;

  public:
    FarFieldOption (std::string outputName, EMSim::GridAngleList angleList, bool storeJones, bool storeMueller, bool noPhi);
    ~FarFieldOption ();

    const std::string& outputName () const { return outputName_; }
    const EMSim::GridAngleList& angleList () const { return angleList_; }
    bool storeJones () const { return storeJones_; }
    bool storeMueller () const { return storeMueller_; }
    bool noPhi () const { return noPhi_; }


    static FarFieldOption parse (const std::string& str);
  };

  template <class ftype>
  class FarFieldCalc {
    STATIC_CLASS (FarFieldCalc);

  public:
    static boost::shared_ptr<std::vector<EMSim::FarFieldEntry<ftype> > > calcEField (const DDAParams<ftype>& ddaParams, FieldCalculator<ftype>& calculator, const EMSim::AngleList& angles, const std::vector<std::complex<ftype> >& pvec, Math::Vector3<ldouble> prop, Math::Vector3<ftype> incPolX, Math::Vector3<ftype> incPolY);

    template <typename MethodType, typename GeometryType>
    static void store (const boost::filesystem::path& outputPrefix, const boost::shared_ptr<GeometryType>& geometry, const boost::shared_ptr<EMSim::DataFiles::Parameters<MethodType> >& parameters, const boost::shared_ptr<EMSim::DataFiles::JonesFarField<ftype> >& farField, const FarFieldOption& option, bool writeTxt);
    static void calcAndStore (const boost::filesystem::path& outputPrefix, const DDAParams<ftype>& ddaParams, FieldCalculator<ftype>& calculator, const boost::shared_ptr<EMSim::DataFiles::Parameters<EMSim::DataFiles::DDAParameters> >& parameters, const std::vector<std::complex<ftype> >& res1, const std::vector<std::complex<ftype> >& res2, bool symmetric, const Beam<ftype>& beam, const EMSim::AngleList& angleList, const std::vector<FarFieldOption>& options, bool writeTxt);
  };

  template <typename ftype>
  template <typename MethodType, typename GeometryType>
  void FarFieldCalc<ftype>::store (const boost::filesystem::path& outputPrefix, const boost::shared_ptr<GeometryType>& geometry, const boost::shared_ptr<EMSim::DataFiles::Parameters<MethodType> >& parameters, const boost::shared_ptr<EMSim::DataFiles::JonesFarField<ftype> >& farField, const FarFieldOption& option, bool writeTxt) {
    if (option.storeJones ()) {
      EMSim::JonesCalculus<ftype>::store (outputPrefix.parent_path () / (outputPrefix.BOOST_FILENAME_STRING + option.outputName () + ".hdf5"), geometry, parameters, farField);
      if (writeTxt)
        EMSim::JonesCalculus<ftype>::storeTxt (outputPrefix.parent_path () / (outputPrefix.BOOST_FILENAME_STRING + option.outputName () + ".txt"), farField, !option.noPhi ());
    }
    if (option.storeMueller ()) {
      boost::shared_ptr<EMSim::DataFiles::MuellerFarField<ftype> > muellerFarField = EMSim::MuellerCalculus<ftype>::computeMuellerFarField (farField);
      EMSim::MuellerCalculus<ftype>::store (outputPrefix.parent_path () / (outputPrefix.BOOST_FILENAME_STRING + option.outputName () + ".mueller.hdf5"), geometry, parameters, muellerFarField);
      if (writeTxt)
        EMSim::MuellerCalculus<ftype>::storeTxt (outputPrefix.parent_path () / (outputPrefix.BOOST_FILENAME_STRING + option.outputName () + ".mueller.txt"), muellerFarField, option.noPhi ());
    }
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(DISABLE_TEMPLATE_INSTANCE, FarFieldCalc)
}

#endif // !DDA_FARFIELDCALC_HPP_INCLUDED
