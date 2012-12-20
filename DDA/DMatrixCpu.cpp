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

#include "DMatrixCpu.hpp"

#include <Core/ProgressBar.hpp>
#include <Core/OStream.hpp>
#include <Core/Allocator.hpp>

#include <DDA/Beam.hpp>

static const bool use128BitAlignment = true;

namespace DDA {
  namespace {
    template <typename T> static void transpose (const boost::const_multi_array_ref<T, 2>& in, boost::multi_array_ref<T, 2>& out) {
      ASSERT (in.data () != out.data ());
      ASSERT (in.shape ()[0] == out.shape ()[1]);
      ASSERT (in.shape ()[1] == out.shape ()[0]);

      for (size_t i = 0; i < in.shape ()[0]; i++)
        for (size_t j = 0; j < in.shape ()[1]; j++)
          out[j][i] = in[i][j];
    }
    template <typename T, size_t dim> static void fill (boost::multi_array_ref<T, dim>& array, const T& value) {
      std::fill (array.data (), array.data () + array.num_elements (), value);
    }
  }

  template <class ftype> static inline std::complex<ftype> getSingleInteractionTerm (const DDAParams<ftype>& ddaParams, Math::Vector3<ftype> r, int mu, int nu) {
    typedef std::complex<ftype> ctype;

    ftype rr = std::sqrt (r * r);
    Math::Vector3<ftype> q = r / rr;
    ftype kr = ddaParams.waveNum () * rr;
    ftype kr2 = kr * kr;
    ftype qmunu = q[mu] * q[nu];
    ctype expval = exp (ctype (0, kr)) / ctype (std::pow (rr, FPConst<ftype>::three));
    ctype br = ctype (3 - kr2, -3 * kr) * qmunu;
    if (mu == nu)
      br += ctype (kr2 - 1, kr);

    return expval * br;
  }

  template <class T> inline std::complex<T> DMatrixCpu<T>::getInteractionTerm (const DDAParams<T>& ddaParams, int i, int j, int k, int mu, int nu, const boost::shared_ptr<const Beam<T> >& beam) {
    ftype gamma = ddaParams.gamma ();
    ftype maxr = 2 / gamma / ddaParams.waveNum ();
    ftype maxr2 = maxr * maxr;

    Math::Vector3<ftype> r = Math::Vector3<ftype> (static_cast<ftype> (i), static_cast<ftype> (j), static_cast<ftype> (k)) * ddaParams.gridUnit ();
    if (ddaParams.periodicityDimension () == 0) {
      if (!i && !j && !k)
        return ctype (0);
      return getSingleInteractionTerm (ddaParams, r, mu, nu);
    } else if (ddaParams.periodicityDimension () == 1) {
      // See doi:10.1364/josaa.25.002693 for information about periodic targets
      ftype phaseShift1 = beam->getPhaseShift (ddaParams, ddaParams.periodicity1 ());
      ftype periodicity1Length = std::sqrt (ddaParams.periodicity1 ().x () * ddaParams.periodicity1 ().x () + ddaParams.periodicity1 ().y () * ddaParams.periodicity1 ().y () + ddaParams.periodicity1 ().y () * ddaParams.periodicity1 ().z ());
      int64_t maxd = static_cast<int64_t> (maxr / periodicity1Length) + 1;
      ctype result = 0;
      for (int64_t m = -maxd; m <= maxd; m++) {
        Math::Vector3<ftype> r2 = r + static_cast<ftype> (m) * ddaParams.periodicity1 ();
        ftype rl = r2.x () * r2.x () + r2.y () * r2.y () + r2.z () * r2.z ();
        if (rl <= maxr2) {
          ftype sup = gamma * gamma * ddaParams.waveNum () * ddaParams.waveNum () * rl;
          sup = -sup * sup; // -(gamma*waveNum*|r|)^4
          ctype factor = std::exp (ctype (sup, static_cast<ftype> (m) * phaseShift1));
          if (i || j || k || m)
            result += factor * getSingleInteractionTerm (ddaParams, r2, mu, nu);
        }
      }
      return result;
    } else if (ddaParams.periodicityDimension () == 2) {
      // See doi:10.1364/josaa.25.002693 for information about periodic targets
      ftype phaseShift1 = beam->getPhaseShift (ddaParams, ddaParams.periodicity1 ());
      ftype phaseShift2 = beam->getPhaseShift (ddaParams, ddaParams.periodicity2 ());
      ftype periodicity1Length = std::sqrt (ddaParams.periodicity1 ().x () * ddaParams.periodicity1 ().x () + ddaParams.periodicity1 ().y () * ddaParams.periodicity1 ().y () + ddaParams.periodicity1 ().y () * ddaParams.periodicity1 ().z ());
      ftype periodicity2Length = std::sqrt (ddaParams.periodicity2 ().x () * ddaParams.periodicity2 ().x () + ddaParams.periodicity2 ().y () * ddaParams.periodicity2 ().y () + ddaParams.periodicity2 ().y () * ddaParams.periodicity2 ().z ());
      int64_t maxd1 = static_cast<int64_t> (maxr / periodicity1Length) + 1;
      int64_t maxd2 = static_cast<int64_t> (maxr / periodicity2Length) + 1;
      ctype result = 0;
      //Core::OStream::getStdout () << maxd1 << std::endl;
      //Core::OStream::getStdout () << maxd2 << std::endl;
      for (int64_t m = -maxd1; m <= maxd1; m++) {
        for (int64_t n = -maxd2; n <= maxd2; n++) {
          Math::Vector3<ftype> r2 = r + static_cast<ftype> (m) * ddaParams.periodicity1 () + static_cast<ftype> (n) * ddaParams.periodicity2 ();
          ftype rl = r2.x () * r2.x () + r2.y () * r2.y () + r2.z () * r2.z ();
          if (rl <= maxr2) {
            ftype sup = gamma * gamma * ddaParams.waveNum () * ddaParams.waveNum () * rl;
            sup = -sup * sup; // -(gamma*waveNum*|r|)^4
            if (i || j || k || m || n) {
              ctype factor = std::exp (ctype (sup, static_cast<ftype> (m) * phaseShift1 + static_cast<ftype> (n) * phaseShift2));
              result += factor * getSingleInteractionTerm (ddaParams, r2, mu, nu);
            }
          }
        }
      }
      return result;
    } else {
      ABORT ();
    }
  }

  template <class T> inline uint32_t DMatrixCpu<T>::clip (int32_t n, uint32_t M) {
    if (n < 0)
      return n + M;
    else if ((uint32_t) n >= M)
      return n - M;
    else
      return n;
  }

  template <typename T> void DMatrixCpu<T>::createDMatrix (const DDAParams<T>& ddaParams, const LinAlg::FFTPlanFactory<T>& planFactory, boost::multi_array_ref<Math::SymMatrix3<std::complex<T> >, 3>& dMatrix, const boost::shared_ptr<const Beam<T> >& beam) {
    ASSERT (dMatrix.shape ()[0] == ddaParams.cgridY ());
    ASSERT (dMatrix.shape ()[1] == ddaParams.cgridZ ());
    ASSERT (dMatrix.shape ()[2] == ddaParams.cgridX ());
    //boost::multi_array<Math::SymMatrix3<ctype>, 3> dMatrix (ddaParams.cgridY (), ddaParams.cgridZ (), ddaParams.cgridX ());

#define MAX(x, y) ((x) > (y) ? (x) : (y))
    typedef Core::Allocator<ctype, MAX (boost::alignment_of<ctype>::value, 16)> Allocator;
#undef MAX

    boost::multi_array<ctype, 3, Allocator> d2Matrix (boost::extents[ddaParams.gridX ()][ddaParams.gridY ()][ddaParams.gridZ ()], boost::fortran_storage_order ());
    boost::multi_array<ctype, 2, Allocator> slice (boost::extents[ddaParams.gridZ ()][ddaParams.gridY ()], boost::fortran_storage_order ());
    boost::multi_array<ctype, 2, Allocator> slice_tr (boost::extents[ddaParams.gridY ()][ddaParams.gridZ ()], boost::fortran_storage_order ());

    boost::shared_ptr<LinAlg::FFTPlan<ftype> > planXf_Dm = planFactory.createPlan (ddaParams.cgridX (), ddaParams.gridSize ().y () () * ddaParams.gridSize ().z () (), true, false, true, false, use128BitAlignment);
    boost::shared_ptr<LinAlg::FFTPlan<ftype> >  planZf_Dm = planFactory.createPlan (ddaParams.cgridZ (), ddaParams.cgridY (), true, false, true, false, use128BitAlignment);
    boost::shared_ptr<LinAlg::FFTPlan<ftype> >  planYf_Dm = planFactory.createPlan (ddaParams.cgridY (), ddaParams.cgridZ (), true, false, true, false, use128BitAlignment);

    cuint64_t overall = (ddaParams.dipoleGeometry ().box ().x () * 2 - 1) * (ddaParams.dipoleGeometry ().box ().y () * 2 - 1) * ddaParams.cgridZ ();

    Core::ProgressBar progress (Core::OStream::getStderr (), 0);
    for (int component = 0; component < 6; component++) {
      int mu, nu;
      switch (component) {
      case 0: mu = 0; nu = 0; break;
      case 1: mu = 0; nu = 1; break;
      case 2: mu = 0; nu = 2; break;
      case 3: mu = 1; nu = 1; break;
      case 4: mu = 1; nu = 2; break;
      case 5: mu = 2; nu = 2; break;
      default: ABORT ();
      }
      fill<ctype> (d2Matrix, 0);
      double prog = 1.0 / 6 * component;
      cuint64_t current = 0;
      progress.reset (overall (), prog, prog + 1.0 / 12);
      for (int k = 0; k < ddaParams.cgridZ (); k++) {
        int kcor = k > ddaParams.gridSize ().z () / 2 ? k - (int)ddaParams.gridZ () : k;
        for (int j = 1 - ddaParams.dipoleGeometry ().box ().y () (); j < ddaParams.dipoleGeometry ().box ().y (); j++)
          for (int i = 1 - ddaParams.dipoleGeometry ().box ().x () (); i < ddaParams.dipoleGeometry ().box ().x (); i++) {
            current++;
            if (current == 1 || current % 1000 == 0)
              progress.update (current - 1, Core::sprintf ("%s / 6: %s / %s", component + 1, current, overall));
            d2Matrix[clip (i, ddaParams.gridX ())][clip (j, ddaParams.gridY ())][k] = getInteractionTerm (ddaParams, i, j, kcor, mu, nu, beam);
          }
      }
      progress.reset (1, prog + 2.0 / 24, prog + 3.0 / 24);
      progress.update (0, Core::sprintf ("%s / 6: FFT X", component + 1));
      planXf_Dm->fftInPlace (d2Matrix.data ());
      progress.reset (ddaParams.cgridX (), prog + 3.0 / 24, prog + 4.0 / 24);
      for (int i = 0; i < ddaParams.cgridX (); i++) {
        if (i == 0 || i % 10 == 9)
          progress.update (i, Core::sprintf ("%s / 6: FFT Y/Z %d / %d", component + 1, i + 1, ddaParams.cgridX ()));
        fill<ctype> (slice, 0);
        for (int j = 1 - ddaParams.dipoleGeometry ().box ().y () (); j < ddaParams.dipoleGeometry ().box ().y (); j++) {
          for (int k = 1 - ddaParams.dipoleGeometry ().box ().z () (); k < ddaParams.dipoleGeometry ().box ().z (); k++) {
            int x = i;
            int y = clip (j, ddaParams.gridY ());
            int z = clip (k, ddaParams.gridZ ());
            slice[z][y] = d2Matrix[x][y][z];
          }
        }
        planZf_Dm->fftInPlace (slice);
        transpose<ctype> (slice, slice_tr);
        planYf_Dm->fftInPlace (slice_tr);
        for (int k = 0; k < ddaParams.cgridZ (); k++)
          for (int j = 0; j < ddaParams.cgridY (); j++)
            dMatrix[j][k][i][component] = slice_tr[j][k] / -(ftype)(ddaParams.gridSize ().x () () * ddaParams.gridSize ().y () () * ddaParams.gridSize ().z () ());
      }
    }
    progress.finish ("DMatrix done");
    progress.cleanup ();
  }

  CALL_MACRO_FOR_DEFAULT_FP_TYPES(CREATE_TEMPLATE_INSTANCE, DMatrixCpu)
}
