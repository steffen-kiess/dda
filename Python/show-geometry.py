#!/usr/bin/env python
# -*- coding: utf-8 -*-
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

# Show geometries
#
# Usage: show-geometry.py <directory or file>
#
# Example: Python/show-geometry.py DDA/output/latest

import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
import sys
import os
import math

havehdf5 = False
try:
    import h5py
    havehdf5 = True
except:
    pass

if not havehdf5:
    print 'Warning: h5py not found'

if havehdf5:
    h5py.get_config ().complex_names = ('real', 'imag')

filename = sys.argv[1]
if os.path.isdir (filename):
    filename = os.path.join (filename, 'Geometry.hdf5')

file = open (filename, 'rb')
str = file.read (4)
del file
hdf = len (str) == 4 and ((ord(str[0]) == 137 and str[1:4] == 'HDF') or str[0:3] == 'MAT')
if not hdf:
    #data = np.recfromtxt (filename, names=True)
    data = np.recfromtxt (filename)
else:
    import h5py
    file = h5py.File (filename, 'r')
    data = np.asarray (file['Geometry/DipolePositions']).view (np.recarray)

fig = plt.figure()
#fig.subplots_adjust (left=0, bottom=0, right=1, top=1)
ax = fig.add_subplot(111, projection='3d')

sc = ax.scatter (data[:,0], data[:,1], data[:,2], c='r', marker='o')
#ax.axis ('equal')
#ax.set_aspect((2.0 * rad + 2) / (rad + 1 + max (nrx, nry) * 10 + rad -9))

#ax.grid (False)

plt.show()
