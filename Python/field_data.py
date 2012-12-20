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

import numpy as np
import matplotlib.pyplot as plt
import os
import sys
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

class FieldData:
    @staticmethod
    def open (name):
        if os.path.isfile (name):
            return FieldData (name)
        if not os.path.isdir (name):
            raise IOError ('Could find directory %s' % name)
        if os.path.exists (os.path.join (name, 'FarPlane.hdf5')) and havehdf5:
            return FieldData (os.path.join (name, 'FarPlane.hdf5'))
        elif os.path.exists (os.path.join (name, 'FarPlane.txt')):
            print 'Warning: Could not open FarPlane.hdf5, using FarPlane.txt'
            return FieldData (os.path.join (name, 'FarPlane.txt'))
        else:
            print 'No far field file in %s found' % name
            sys.exit (1)

    def __init__(self, filename):
        self.filename = filename
        file = open (filename, 'rb')
        str = file.read (4)
        del file
        hdf = len (str) == 4 and ((ord(str[0]) == 137 and str[1:4] == 'HDF') or str[0:3] == 'MAT')
        if not hdf:
            #self.data = np.recfromtxt (filename, dtype='f8', names=True) # Some versions of numpy don't like the dtype
            self.data = np.recfromtxt (filename, names=True)
            self.havephi = self.data.dtype.names.count ('phi') != 0
        else:
            import h5py
            file = h5py.File (filename, 'r')
            theta = np.asarray (file['JonesFarField/Theta']) / math.pi * 180
            phi = np.asarray (file['JonesFarField/Phi']) / math.pi * 180
            d = np.asarray (file['JonesFarField/Data'])
            s1 = d[0,:,1,1]
            s2 = d[0,:,0,0]
            s3 = d[0,:,1,0]
            s4 = d[0,:,0,1]
            del d
            self.havephi = any (phi != phi[0])
            self.data = np.rec.array ([theta, phi, s1, s2, s3, s4], names=['theta', 'phi', 's1', 's2', 's3', 's4'])
            del theta, phi, s1, s2, s3, s4
            
            #if self.havephi:
            #    raise Exception ('Trying to read non-plane data')
            
            del file

        if not self.havephi:
            self.data.theta = (self.data.theta + 180) % 360 - 180
            self.data.sort ()
            
            self.theta = self.data.theta
        else:
            self.theta = self.data.theta
            self.phi = self.data.phi

        if hdf:
            self.xx = self.data.s1
            self.yy = self.data.s2
            self.xy = self.data.s3
            self.yx = self.data.s4
        else:
            self.xx = self.data.s1r + self.data.s1i * 1j
            self.yy = self.data.s2r + self.data.s2i * 1j
            self.xy = self.data.s3r + self.data.s3i * 1j
            self.yx = self.data.s4r + self.data.s4i * 1j

        self.ampl = np.sqrt (np.abs (self.xx**2) + np.abs (self.yy**2) + np.abs (self.xy**2) + np.abs (self.yx**2))

    def scatvec (self, x, y):
        len = np.sqrt (np.abs (x * x) + np.abs (y * y))
        x /= len
        y /= len
        #print (x)
        #print (y)
        xres = self.xx * x + self.xy * y
        yres = self.yy * y + self.yx * x
        return xres, yres

    def scat (self, x, y):
        xres, yres = self.scatvec (x, y)
        return np.sqrt (np.abs(xres**2) + np.abs(yres**2))

#fig = plt.figure()
#ax = fig.add_subplot(111)
##ax.plot (data.theta, ampl)
##ax.plot (data.theta, scat (1.0, 0.0))
#ax.plot (data.theta, scat (0.0, 1.0))
#ax.grid (True)
#
#plt.show ()
