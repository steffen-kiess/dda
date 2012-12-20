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

#include <EMSim/Rotation.hpp>

typedef double D;
typedef Math::Quaternion<double> Q;
typedef EMSim::Rotation<double> R;
typedef Math::Vector3<double> V;

static void check (R r) {
  double alpha, beta, gamma;
  double alphaD, betaD, gammaD;
  r.toZYZ (alpha, beta, gamma);
  r.toZYZDeg (alphaD, betaD, gammaD);
  R r2 = R::fromZYZ (alpha, beta, gamma);
  double sd = r.squaredDifference (r2);
  Q diff = r.quaternion () - r2.quaternion ();
  //Core::OStream::getStdout () << alphaD << " " << betaD << " " << gammaD << " " << r << " " << r2 << " " << sd << std::endl;
  //Core::OStream::getStdout () << diff << std::endl;
  //Core::OStream::getStdout () << diff.squaredNorm () << std::endl;
  //Core::OStream::getStdout () << r.squaredDifference (r2) << std::endl;
  ASSERT (sd < 1e-15);
  /*
  */
}

int main () {
  for (int alpha = 0; alpha <= 360; alpha += 10)
    for (int beta = 0; beta <= 360; beta += 10)
      for (int gamma = 0; gamma <= 360; gamma += 10)
        //check (R::fromZYZDeg (alpha, beta + 0.08, gamma));
        check (R::fromZYZDeg (alpha, beta, gamma));
}
