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

# This script will run DDA with the given parameters, run adda with the same
# parameters and compare the results.
#
# Parameters to this script should be adda-compatible parameters, see
# test-adda.sh for examples.
#
# Return value is 0 if everything succeeded and the results match and non-zero
# otherwise.
#
# This script also tests Hdf5Util jones-to-mueller and Hdf5Util hdf5-to-text.

set -e

POS="${0%/*}"

ADDA="adda"

ADDASC="$POS/adda_scat_params.dat"
DDA="$POS/DDA"
HDF5UTIL="$POS/../EMSim/Hdf5Util"
CMP="$POS/FieldDiff"
OUT1="$POS/output/${DDA_PREFIX}compare1"
OUT2="$POS/output/${DDA_PREFIX}compare2"

rm -rf "$OUT1" "$OUT2"

mkdir -p "$POS/output"

set -x
"$DDA" $DDA_ADDITIONAL_OPTIONS --adda-options -- -dir "$OUT1" "$@"
"$ADDA" -scat_grid_inp "$ADDASC" -dir "$OUT2" "$@"
set +x

for j in "CrossSec-Pol1" "CrossSec-Pol2"; do
    i="$OUT1/$j.txt"
    if [ -f "$i" ]; then
        mv "$OUT1/$j.txt" "$OUT1/$j.2.txt"
        "$HDF5UTIL" --verbose hdf5-to-text "$OUT1/$j.hdf5"
        cmp "$OUT1/$j.txt" "$OUT1/$j.2.txt"
        cat "$i" | grep '\(ext\|abs\)' | ( echo 'Cxxx Qxxx'; while read a1 b1 c1; read a2 b2 c2; do echo "$c1 $c2"; done ) > "$OUT1/P-$j"
    fi
done

for j in "CrossSec-Y" "CrossSec-X"; do
    i="$OUT2/$j"
    if [ -f "$i" ]; then
        ( echo 'Cxxx Qxxx'; while read a1 b1 c1; read a2 b2 c2; do echo "$c1 $c2"; done ) < "$i" > "$OUT2/P-$j"
    fi
done

haveScatGrid=0
for i in "$@"; do
    if [ "$i" = "-store_scat_grid" ]; then
        haveScatGrid=1
    fi
done

for i in IncBeam-X.txt IncBeam-Y.txt DipPol-X.txt DipPol-Y.txt IntField-X.txt IntField-Y.txt ampl/FarPlane.txt ampl_scatgrid/FarGrid.txt mueller/FarPlane.mueller.txt mueller_scatgrid/FarGrid.mueller.txt P-CrossSec-X P-CrossSec-Y; do
    d="${i#*/}"
    a="${i%/*}"
    a="${a%.txt}"
    d="$(printf '%s\n' $d | sed 's/-Y/-Pol1/g;s/-X/-Pol2/g')"
    if [ "$d" != "${d%.txt}" -a -f "$OUT1/$d" ]; then
        OPT=
        if [ "$d" != "${d#FarPlane}" ]; then
            OPT="--no-phi"
        fi
        "$HDF5UTIL" --verbose hdf5-to-text $OPT -o "$OUT1/$d.new" "$OUT1/${d%.txt}.hdf5"
        FC="$(head -2 "$OUT1/$d.new" | tail -1 | sed 's/[^ ]//g' | wc -c)"
        echo Comparing "$OUT1/$d.new" and "$OUT1/$d" with "$FC:1"
        "$CMP" --max-error 1e-6 "$OUT1/$d.new" "$OUT1/$d" "$FC:1"
        if [ "$DDA_NO_EXACT_CMP" != "1" ]; then
            cmp "$OUT1/$d.new" "$OUT1/$d"
        fi
        if [ "$d" != "${d%.mueller.txt}" ]; then
            "$HDF5UTIL" --verbose jones-to-mueller "$OUT1/${d%.mueller.txt}.hdf5" -o "$OUT1/${d%.mueller.txt}.mueller.new.hdf5"
            h5dump "$OUT1/${d%.mueller.txt}.mueller.hdf5" | sed 1d > "$OUT1/${d%.mueller.txt}.mueller.hdf5.dump"
            h5dump "$OUT1/${d%.mueller.txt}.mueller.new.hdf5" | sed 1d > "$OUT1/${d%.mueller.txt}.mueller.new.hdf5.dump"
            if [ "$DDA_NO_EXACT_CMP" != "1" ]; then
                cmp "$OUT1/${d%.mueller.txt}.mueller.hdf5.dump" "$OUT1/${d%.mueller.txt}.mueller.new.hdf5.dump"
            fi
            "$HDF5UTIL" --verbose hdf5-to-text $OPT "$OUT1/${d%.mueller.txt}.mueller.new.hdf5"
            echo Comparing "$OUT1/$d.new" and "$OUT1/${d%.mueller.txt}.mueller.new.txt" with "$FC:1"
            "$CMP" --max-error 1e-6 "$OUT1/$d.new" "$OUT1/${d%.mueller.txt}.mueller.new.txt" "$FC:1"
            if [ "$DDA_NO_EXACT_CMP" != "1" ]; then
                cmp "$OUT1/$d.new" "$OUT1/${d%.mueller.txt}.mueller.new.txt"
            fi
        fi
    fi
    if [ -f "$OUT2/$a" ]; then
        if [ ! -f "$OUT1/$d" ]; then
            if [ "${d%.txt}" != "$d" ]; then
                d="${d%-Pol1.txt}.txt"
            else
                d="${d%-Pol1}"
            fi
        fi
        if [ "$haveScatGrid" = 1 -a ! -f "$OUT1/$d" -a "$d" = "P-CrossSec-Pol2" ]; then
            d="${d%-Pol2}" # Adda creates a CrossSec-X file when store_scat_grid is passed to adda
        fi
        if [ "$a" != "${a%-X}" -a "$haveScatGrid" = 1 -a ! -f "$OUT1/$d" ]; then
            continue # Adda uses two polarizations when store_scat_grid is given
        fi
        if [ ! -f "$OUT1/$d" ]; then
            echo "File $OUT1/$d (needed to compare to $OUT2/$a ) not found"
            false
        fi
        if [ "${d#Far}" != "$d" ]; then
            IS_SYM="$(echo "$(sed 's/ .*//' < "$OUT2/$a" | LC_ALL=C sort -n | tail -1) < 180.01" | bc -l)"
            if [ "$IS_SYM" = 1 ]; then
                # Remove angles > 180
                grep -E '^#|^[0-9]\.|^[0-9][0-9]\.|^1[0-7][0-9]\.|^180\.0* ' < "$OUT1/$d" > "$OUT1/${d%.txt}.half.txt"
                d="${d%.txt}.half.txt"
            fi
        fi
        K="$(grep '^Wavenumber: ' "$OUT1/log" | sed 's/^Wavenumber: //;s/ .*//')"
        FarFact="$(echo "-(10^-6) / $K" | bc -l)"
        FarFact2="$(echo "$FarFact * $FarFact" | bc -l)"
        PAR=
        if [ "${d#FarGrid.mueller.}" != "$d" ]; then
            PAR="2:1 16:$FarFact2"
        elif [ "${d#FarGrid.}" != "$d" ]; then
            PAR="2:1 8:$FarFact"
        elif [ "${d#FarPlane.mueller.}" != "$d" ]; then
            PAR="1:1 16:$FarFact2"
        elif [ "${d#FarPlane.}" != "$d" ]; then
            PAR="1:1 8:$FarFact"
        elif [ "${d#P-Cross}" != "$d" ]; then
            PAR="1:1e-12 1:1"
        elif [ "${d#DipPol}" != "$d" ]; then
            PAR="3:1e-6 1:1e-36 6:1e-18"
        else
            PAR="3:1e-6 1:1 6:1"
        fi
        echo Compare "$OUT2/$a" "$OUT1/$d" with "$PAR" "DDA_MAX_ERROR=$DDA_MAX_ERROR"
        DDA_MAX_ERROR_ARG=
        if [ "$DDA_MAX_ERROR" != "" ]; then
            DDA_MAX_ERROR_ARG="--max-error $DDA_MAX_ERROR"
        fi
        echo "$CMP" $DDA_MAX_ERROR_ARG "$OUT2/$a" "$OUT1/$d" $PAR
        "$CMP" $DDA_MAX_ERROR_ARG "$OUT2/$a" "$OUT1/$d" $PAR
    fi
done

echo SUCCESS

#cat "$OUT1/CrossSec"*
#echo ----
#cat "$OUT2/CrossSec"*
