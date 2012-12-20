#!/bin/sh
#
# Copyright (c) 2010-2012 Steffen Kieß
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

# Compare DDA and adda for various options

set -e

POS="${0%/*}"
cd "$POS"
POS=.

exec 2>&1

echo "Running $0 $*"

DEFOPT="-store_beam -store_dip_pol -store_int_field"

export DDA_PREFIX=
if [ "$1" = "--prefix" ]; then
    export DDA_PREFIX="$2-"
    shift
    shift
fi

export DDA_NO_EXACT_CMP=0
if [ "$1" = "--no-exact-cmp" ]; then
    export DDA_NO_EXACT_CMP=1
    shift
fi

export DDA_MAX_ERROR=
if [ "$1" = "--max-error" ]; then
    export DDA_MAX_ERROR="$2"
    shift
    shift
fi

export DDA_ADDITIONAL_OPTIONS="$*"

#BOX_10_8_6="-shape box 0.8 0.6 -size 10"
BOX_10_8_6="-shape read ./box_10_8_6.geom"

set -x

"$POS/compare-adda.sh" $DEFOPT -dpl 15

"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -scat_matr both
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -scat_matr both -prop 1 -2 3
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -store_scat_grid -scat_matr both
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -store_scat_grid -scat_matr both
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -store_scat_grid -scat_matr both -prop 1 -2 3

"$POS/compare-adda.sh" $DEFOPT -dpl 15 -m 1.4 0.2
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -m 1.4 0.2
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -store_scat_grid -scat_matr both -prop 1 -2 3 -m 1.4 0.2

# -orient
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -orient 10 30 50
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -orient 40 -50 3
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -store_scat_grid -orient 13 41 100
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -store_scat_grid -scat_matr both -prop 1 -2 3 -orient 13 41 100
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -store_scat_grid -scat_matr both -prop 10 -2 3 -orient -5 -30 10 -iter bicgstab # doesn't work with qmrcs

# -beam
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -prop 1 2 3 -beam plane
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -prop 1 2 3 -beam lminus 1
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -prop 1 2 3 -beam lminus 10
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -prop 1 2 3 -beam davis3 1
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -prop 1 2 3 -beam davis3 10
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -prop 1 2 3 -beam barton5 1
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -prop 1 2 3 -beam barton5 10
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -prop 1 2 3 -beam barton5 10 0 0 0
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -prop 1 2 3 -beam barton5 10 7 2 4
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -store_scat_grid -scat_matr both -prop 10 -2 3 -orient -5 -30 10 -iter bicgstab -beam barton5 1
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -store_scat_grid -scat_matr both -prop 10 -2 3 -orient -5 -30 10 -iter bicgstab -beam barton5 1 1 6 3

# -pol
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -pol ldr
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -prop 1 2 3 -orient 40 70 100 -beam barton5 10 -pol ldr
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -pol ldr avgpol
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -prop 1 2 3 -orient 40 70 100 -beam barton5 10 -pol ldr avgpol
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -pol cm
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -scat_matr both -prop 1 2 3 -orient 40 70 100 -beam barton5 10 -pol cm

# -eps
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -eps 1

# -iter
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -eps 1 -iter cgnr
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -eps 1 -iter bicg
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -eps 1 -iter bicgstab
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -eps 1 -iter qmr

"$POS/compare-adda.sh" $DEFOPT -dpl 15 -ntheta 1
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -ntheta 2
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -ntheta 3
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -ntheta 4
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -ntheta 5
"$POS/compare-adda.sh" $DEFOPT -dpl 15 -ntheta 142
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -ntheta 1
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -ntheta 2
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -ntheta 3
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -ntheta 4
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -ntheta 5
"$POS/compare-adda.sh" $DEFOPT -lambda 30 $BOX_10_8_6 -dpl 30 -ntheta 142


echo All tests successful
