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

#include "ComplexConversion.hpp"

#include <Core/Memory.hpp>

#include <HDF5/Type.hpp>

#include <cstring>

namespace HDF5 {
  namespace {
    // http://www.hdfgroup.org/HDF5/doc/RM/RM_H5T.html#Datatype-Register
    // http://www.hdfgroup.org/HDF5/doc/H5.user/Datatypes.html#Datatypes-DataConversion
    // http://www.mail-archive.com/hdf-forum@hdfgroup.org/msg03723.html

    static herr_t convertRealToComplex (hid_t src_id, hid_t dst_id, H5T_cdata_t* cdata, size_t nelmts, size_t buf_stride, UNUSED size_t bkg_stride, void* buf, UNUSED void* bkg, hid_t dset_xfer_plist) throw () {
      struct Priv {
        H5T_conv_t func;
        H5T_cdata_t* cdata;
        size_t srcSize;
        size_t dstSize;
        size_t realOffset;
        size_t imagOffset;
        size_t memberSize;
        HDF5::DataType member;
      };

      switch (cdata->command) {
      case H5T_CONV_INIT: {
        HDF5::DataType src (HDF5::dontTakeOwnership (src_id));
        HDF5::DataType dst (HDF5::dontTakeOwnership (dst_id));

        if (dst.getClass () != H5T_COMPOUND)
          return -1;
        HDF5::CompoundType cDst (dst);
        if (cDst.nMembers () != 2)
          return -1;

        Priv priv;

        if (cDst.memberName (0) != "real")
          return -1;
        priv.realOffset = cDst.memberOffset (0);

        if (cDst.memberName (1) != "imag")
          return -1;
        priv.imagOffset = cDst.memberOffset (1);

        HDF5::DataType rType = cDst.memberType (0);
        HDF5::DataType iType = cDst.memberType (1);
        if (!rType.equals (iType))
          return -1;

        priv.func = H5Tfind (src.handle (), rType.handle (), &priv.cdata);
        if (!priv.func)
          return -1;

        ASSERT (priv.cdata->need_bkg == H5T_BKG_NO);

        priv.member = rType;
        priv.memberSize = rType.getSize ();

        priv.srcSize = src.getSize ();
        priv.dstSize = dst.getSize ();

        cdata->priv = new Priv (priv);
        return 0;
      }

      case H5T_CONV_FREE: {
        Priv* priv = (Priv*) cdata->priv;
        delete priv;
        cdata->priv = NULL;
        return 0;
      }

      case H5T_CONV_CONV: {
        Priv* priv = (Priv*) cdata->priv;

        size_t srcStride = priv->srcSize;
        if (buf_stride)
          srcStride = buf_stride;
        size_t dstStride = priv->dstSize;
        if (buf_stride)
          dstStride = buf_stride;

        char* src;
        char* dst;
        ptrdiff_t srcOffset, dstOffset;
        if (dstStride > srcStride) { // go backwards
          src = (char*) buf + (nelmts - 1) * srcStride;
          dst = (char*) buf + (nelmts - 1) * dstStride;
          srcOffset = -srcStride;
          dstOffset = -dstStride;
        } else { // go forward
          src = (char*) buf;
          dst = (char*) buf;
          srcOffset = srcStride;
          dstOffset = dstStride;
        }

        for (size_t i = 0; i < nelmts; i++, src += srcOffset, dst += dstOffset) {
          char* b = src;
          if (priv->memberSize > priv->srcSize) { // use destination for conversion
            b = dst + priv->realOffset;
            memmove (dst + priv->realOffset, src, priv->srcSize);
          }
          {
            H5T_cdata_t data;
            data.command = H5T_CONV_CONV;
            data.need_bkg = H5T_BKG_NO;
            data.recalc = cdata->recalc;
            data.priv = priv->cdata->priv;
            if (priv->func (src_id, priv->member.handle (), &data, 1, 0, 0, b, NULL, dset_xfer_plist) < 0) {
              HDF5::Exception::check ("H5Epush2", H5Epush2 (H5E_DEFAULT, __FILE__, __FUNCTION__, __LINE__, H5E_ERR_CLS, H5E_DATATYPE, H5E_CANTINIT, "unable to convert complex datatype member"));
              return -1;
            }
          }
          if (priv->memberSize <= priv->srcSize) { // used source for conversion
            memmove (dst + priv->realOffset, src, priv->memberSize);
          }

          memset (dst + priv->imagOffset, 0, priv->memberSize);
        }

        return 0;
      }

      default:
        HDF5::Exception::check ("H5Epush2", H5Epush2 (H5E_DEFAULT, __FILE__, __FUNCTION__, __LINE__, H5E_ERR_CLS, H5E_DATATYPE, H5E_UNSUPPORTED, "unknown conversion command"));
        return -1;
      }
    }

    static herr_t convertComplexToReal (hid_t src_id, hid_t dst_id, H5T_cdata_t* cdata, size_t nelmts, size_t buf_stride, UNUSED size_t bkg_stride, void* buf, UNUSED void* bkg, hid_t dset_xfer_plist) throw () {
      struct Priv {
        H5T_conv_t func;
        H5T_cdata_t* cdata;
        size_t srcSize;
        size_t dstSize;
        size_t realOffset;
        size_t imagOffset;
        size_t memberSize;
        HDF5::DataType member;
        H5T_conv_t dfunc;
        H5T_cdata_t* dcdata;
      };

      switch (cdata->command) {
      case H5T_CONV_INIT: {
        HDF5::DataType src (HDF5::dontTakeOwnership (src_id));
        HDF5::DataType dst (HDF5::dontTakeOwnership (dst_id));

        if (src.getClass () != H5T_COMPOUND)
          return -1;
        HDF5::CompoundType cSrc (src);
        if (cSrc.nMembers () != 2)
          return -1;

        Priv priv;

        if (cSrc.memberName (0) != "real")
          return -1;
        priv.realOffset = cSrc.memberOffset (0);

        if (cSrc.memberName (1) != "imag")
          return -1;
        priv.imagOffset = cSrc.memberOffset (1);

        HDF5::DataType rType = cSrc.memberType (0);
        HDF5::DataType iType = cSrc.memberType (1);
        if (!rType.equals (iType))
          return -1;

        priv.func = H5Tfind (rType.handle (), dst.handle (), &priv.cdata);
        if (!priv.func)
          return -1;
        ASSERT (priv.cdata->need_bkg == H5T_BKG_NO);

        priv.dfunc = H5Tfind (iType.handle (), HDF5::getH5Type<double> ().handle (), &priv.dcdata);
        if (!priv.dfunc)
          return -1;
        ASSERT (priv.dcdata->need_bkg == H5T_BKG_NO);

        priv.member = rType;
        priv.memberSize = rType.getSize ();

        priv.srcSize = src.getSize ();
        priv.dstSize = dst.getSize ();

        cdata->priv = new Priv (priv);
        return 0;
      }

      case H5T_CONV_FREE: {
        Priv* priv = (Priv*) cdata->priv;
        delete priv;
        cdata->priv = NULL;
        return 0;
      }

      case H5T_CONV_CONV: {
        Priv* priv = (Priv*) cdata->priv;

        size_t srcStride = priv->srcSize;
        if (buf_stride)
          srcStride = buf_stride;
        size_t dstStride = priv->dstSize;
        if (buf_stride)
          dstStride = buf_stride;

        char* src;
        char* dst;
        ptrdiff_t srcOffset, dstOffset;
        if (dstStride > srcStride) { // go backwards
          src = (char*) buf + (nelmts - 1) * srcStride;
          dst = (char*) buf + (nelmts - 1) * dstStride;
          srcOffset = -srcStride;
          dstOffset = -dstStride;
        } else { // go forward
          src = (char*) buf;
          dst = (char*) buf;
          srcOffset = srcStride;
          dstOffset = dstStride;
        }

        for (size_t i = 0; i < nelmts; i++, src += srcOffset, dst += dstOffset) {
          double imag;
          char* b = src + priv->imagOffset;
          if (sizeof (double) > priv->memberSize) {
            b = (char*) &imag;
            memcpy (&imag, src + priv->imagOffset, priv->memberSize);
          }
          {
            H5T_cdata_t data;
            data.command = H5T_CONV_CONV;
            data.need_bkg = H5T_BKG_NO;
            data.recalc = cdata->recalc;
            data.priv = priv->dcdata->priv;
            if (priv->dfunc (priv->member.handle (), HDF5::getH5Type<double> ().handle (), &data, 1, 0, 0, b, NULL, dset_xfer_plist) < 0) {
              HDF5::Exception::check ("H5Epush2", H5Epush2 (H5E_DEFAULT, __FILE__, __FUNCTION__, __LINE__, H5E_ERR_CLS, H5E_DATATYPE, H5E_CANTINIT, "unable to convert complex datatype member"));
              return -1;
            }
          }
          if (sizeof (double) <= priv->memberSize) {
            memcpy (&imag, src + priv->imagOffset, sizeof (double));
          }
          if (imag != 0) {
            HDF5::Exception::check ("H5Epush2", H5Epush2 (H5E_DEFAULT, __FILE__, __FUNCTION__, __LINE__, H5E_ERR_CLS, H5E_DATATYPE, H5E_BADVALUE, "attempting to vonvert complex value with an imaginary part of %e to real value", imag));
            return -1;
          }

          b = src + priv->realOffset;
          if (priv->dstSize > priv->memberSize) { // use destination for conversion
            b = dst;
            memmove (dst, src + priv->realOffset, priv->memberSize);
          }
          {
            H5T_cdata_t data;
            data.command = H5T_CONV_CONV;
            data.need_bkg = H5T_BKG_NO;
            data.recalc = cdata->recalc;
            data.priv = priv->cdata->priv;
            if (priv->func (priv->member.handle (), dst_id, &data, 1, 0, 0, b, NULL, dset_xfer_plist) < 0) {
              HDF5::Exception::check ("H5Epush2", H5Epush2 (H5E_DEFAULT, __FILE__, __FUNCTION__, __LINE__, H5E_ERR_CLS, H5E_DATATYPE, H5E_CANTINIT, "unable to convert complex datatype member"));
              return -1;
            }
          }
          if (priv->dstSize <= priv->memberSize) { // used source for conversion
            memmove (dst, src + priv->realOffset, priv->dstSize);
          }
        }

        return 0;
      }

      default:
        HDF5::Exception::check ("H5Epush2", H5Epush2 (H5E_DEFAULT, __FILE__, __FUNCTION__, __LINE__, H5E_ERR_CLS, H5E_DATATYPE, H5E_UNSUPPORTED, "unknown conversion command"));
        return -1;
      }
    }

    class ComplexConversionRegistration {
    public:
      ComplexConversionRegistration () {
        HDF5::DataType compoundType = HDF5::CompoundType::create (1);
        HDF5::DataType intType = HDF5::NATIVE_INT32 ();
        HDF5::DataType floatType = HDF5::NATIVE_FLOAT ();

        HDF5::Exception::check ("H5Tregister", H5Tregister (H5T_PERS_SOFT, "convertRealToComplex (int)", intType.handle (), compoundType.handle (), convertRealToComplex));
        HDF5::Exception::check ("H5Tregister", H5Tregister (H5T_PERS_SOFT, "convertRealToComplex (float)", floatType.handle (), compoundType.handle (), convertRealToComplex));

        HDF5::Exception::check ("H5Tregister", H5Tregister (H5T_PERS_SOFT, "convertComplexToReal (int)", compoundType.handle (), intType.handle (), convertComplexToReal));
        HDF5::Exception::check ("H5Tregister", H5Tregister (H5T_PERS_SOFT, "convertComplexToReal (float)", compoundType.handle (), floatType.handle (), convertComplexToReal));
      }
      ~ComplexConversionRegistration () {
      }
    };
  }

  void registerComplexConversion () {
    static ComplexConversionRegistration reg;
  }
}
