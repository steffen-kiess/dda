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

#include "MatVecCpu.hpp"

namespace DDA {
  static const bool use128BitAlignment = true;

  namespace {
    template <typename T> static void transpose (const boost::const_multi_array_ref<T, 3>& in, boost::multi_array_ref<T, 3>& out, size_t index) {
      ASSERT (in.data () != out.data ());
      ASSERT (in.shape ()[0] == out.shape ()[1]);
      ASSERT (in.shape ()[1] == out.shape ()[0]);
      ASSERT (index < in.shape ()[2]);
      ASSERT (index < out.shape ()[2]);

      for (size_t i = 0; i < in.shape ()[0]; i++)
        for (size_t j = 0; j < in.shape ()[1]; j++)
          out[j][i][index] = in[i][j][index];
    }
    template <typename T, size_t dim> static void fill (boost::multi_array_ref<T, dim>& array, const T& value) {
      std::fill (array.data (), array.data () + array.num_elements (), value);
    }
  }

  template <class F> MatVecCpu<F>::MatVecCpu (const DDAParams<ftype>& ddaParams, const boost::const_multi_array_ref<Math::SymMatrix3<ctype>, 3>& Dmatrix, const LinAlg::FFTPlanFactory<ftype>& planFactory) :
    MatVec<F> (ddaParams),
    Dmatrix_ (Dmatrix),
    Xmatrix (boost::extents[g ().gridX ()][g ().dipoleGeometry ().box ().y () ()][g ().dipoleGeometry ().box ().z () ()][3], boost::fortran_storage_order ()),
    slices (boost::extents[g ().gridZ ()][g ().gridY ()][3], boost::fortran_storage_order ()),
    slices_tr (boost::extents[g ().gridY ()][g ().gridZ ()][3], boost::fortran_storage_order ()),
    // times = Xmatrix.shape ()[2] * 3, stride = Xmatrix.sizeY * Xmatrix.sizeX
    planX (planFactory.createPlan (g ().cgridX (), g ().dipoleGeometry ().box ().y (), true, false, true, true, use128BitAlignment)),
    // times = 3, stride = gridY * gridZ
    planZ (planFactory.createPlan (g ().cgridZ (), g ().dipoleGeometry ().box ().y (), true, false, true, true, use128BitAlignment)),
    // times = 1
    planY (planFactory.createPlan (g ().cgridY (), g ().cgridZ () * 3, true, false, true, true, use128BitAlignment))
  {
  }
  template <class F> MatVecCpu<F>::~MatVecCpu () {}

  namespace {
    template <class F> inline F maybeConj (F f, bool c) {
      return c ? conj (f) : f;
    }
  }

  template <class F> void MatVecCpu<F>::apply (const std::vector<ctype>& arg, std::vector<ctype>& result, bool conj, Core::ProfilingDataPtr prof) {
    const DDAParams<ftype>& g = this->ddaParams ();

    fill<ctype> (Xmatrix, 0);
  
    for (uint32_t i = 0; i < g.cnvCount (); i++) {
      Math::Vector3<uint32_t> pos = dipoleGeometry ().getGridCoordinates (i);
      Math::DiagMatrix3<ctype> cc = this->cc ().cc_sqrt ()[dipoleGeometry ().getMaterialIndex (i)];
      Math::Vector3<ctype> r = cc * maybeConj (g.get (arg, i), conj);
      for (int comp = 0; comp < 3; comp++)
        Xmatrix[pos.x ()][pos.y ()][pos.z ()][comp] = r[comp];
    }

    for (size_t i = 0; i < Xmatrix.shape ()[2] * 3; i++) {
      Core::ProfileHandle _p1 (prof, "fft" /* "planXf" */);
      planX->fftInPlace (Xmatrix.data () + Xmatrix.shape ()[1] * Xmatrix.shape ()[0] * i);
    }

    {
      Core::ProfileHandle _p_ (prof, "il");
      for (size_t i = 0; i < g.cgridX (); i++) {
        fill<ctype> (slices, 0);
        for (size_t j = 0; j < g.dipoleGeometry ().box ().y (); j++)
          for (size_t k = 0; k < g.dipoleGeometry ().box ().z (); k++)
            for (int comp = 0; comp < 3; comp++)
              slices[k][j][comp] = Xmatrix[i][j][k][comp];
        for (size_t comp = 0; comp < 3; comp++) {
          Core::ProfileHandle _p1 (prof, "fft" /* "planZf" */);
          planZ->fftInPlace (slices.data () + comp * g.gridY () * g.gridZ ());
        }
        for (int comp = 0; comp < 3; comp++)
          transpose<ctype> (slices, slices_tr, comp);
        {
          Core::ProfileHandle _p1 (prof, "fft" /* "planYf" */);
          planY->fftInPlace (slices_tr.data ());
        }
        {
          Core::ProfileHandle _p1 (prof, "iil");
          for (size_t k = 0; k < g.cgridZ (); k++) {
            for (size_t j = 0; j < g.cgridY (); j++) {
              Math::Vector3<ctype> xv;
              for (int comp = 0; comp < 3; comp++)
                xv[comp] = slices_tr[j][k][comp];
              Math::Vector3<ctype> yv = dMatrix ()[j][k][i] * xv;
              for (int comp = 0; comp < 3; comp++)
                slices_tr[j][k][comp] = yv[comp];
            }
          }
        }
        {
          Core::ProfileHandle _p1 (prof, "fft" /* "planYb" */);
          planY->ifftInPlace (slices_tr.data ());
        }
        for (int comp = 0; comp < 3; comp++)
          transpose<ctype> (slices_tr, slices, comp);
        for (size_t comp = 0; comp < 3; comp++) {
          Core::ProfileHandle _p1 (prof, "fft" /* "planZb" */);
          planZ->ifftInPlace (slices.data () + comp * g.gridY () * g.gridZ ());
        }
        for (size_t j = 0; j < g.dipoleGeometry ().box ().y (); j++)
          for (size_t k = 0; k < g.dipoleGeometry ().box ().z (); k++)
            for (int comp = 0; comp < 3; comp++)
              Xmatrix[i][j][k][comp] = slices[k][j][comp];
      }
    }

    for (size_t i = 0; i < Xmatrix.shape ()[2] * 3; i++) {
      Core::ProfileHandle _p1 (prof, "fft" /* "planXb" */);
      planX->ifftInPlace (Xmatrix.data () + Xmatrix.shape ()[1] * Xmatrix.shape ()[0] * i);
    }

    for (uint32_t i = g.nvCount (); i < g.vecStride (); i++) {
      g.set (result, i, Math::Vector3<ctype> (0, 0, 0));
    }
    for (uint32_t i = 0; i < g.cnvCount (); i++) {
      Math::Vector3<uint32_t> pos = dipoleGeometry ().getGridCoordinates (i);
      Math::Vector3<ctype> r;
      for (int comp = 0; comp < 3; comp++)
        r[comp] = Xmatrix[pos.x ()][pos.y ()][pos.z ()][comp];
      Math::Vector3<ctype> a = maybeConj (g.get (arg, i), conj);
      Math::Vector3<ctype> r2 = this->cc ().cc_sqrt ()[dipoleGeometry ().getMaterialIndex (i)] * r + a;
      g.set (result, i, maybeConj (r2, conj));
    }
  }


  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, MatVecCpu)
}
