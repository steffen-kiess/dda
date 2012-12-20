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

import sys
import os
import gtk
import pango

import numpy as np
import matplotlib
from matplotlib.figure import Figure
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas

def cround (x, digits):
    return round (x.real, digits) + round (x.imag, digits) * 1j

class ComplexEntry(gtk.Table):
    def __init__ (self, min, max, step=0.05, digits=2):
        gtk.Table.__init__ (self, 2, 2)

        self.min = min
        self.max = max
        self.digits = digits
        self.handlers = []

        labelR = gtk.Label ("Real")
        self.attach (labelR, 0, 1, 0, 1)

        self.spinR = gtk.SpinButton (digits=digits)
        self.spinR.set_range (min.real, max.real)
        self.spinR.set_increments (step, step)
        self.spinR.connect ("output", self.handleUpdate)
        self.attach (self.spinR, 1, 2, 0, 1)

        labelI = gtk.Label ("Imag")
        self.attach (labelI, 0, 1, 1, 2)

        self.spinI = gtk.SpinButton (digits=digits)
        self.spinI.set_range (min.imag, max.imag)
        self.spinI.set_increments (step, step)
        self.spinI.connect ("output", self.handleUpdate)
        self.attach (self.spinI, 1, 2, 1, 2)

        self.setValue (0j)

        self.show_all ()

    def handleUpdate (self, spin):
        value = round (self.spinR.get_value (), self.digits) + round (self.spinI.get_value (), self.digits) * 1j
        needUpdate = value != self.value and not self.inUpdate
        self.value = value
        if needUpdate:
            for handler in self.handlers:
                handler (value)

    def setValue (self, value):
        assert value.real >= self.min.real, value
        assert value.real <= self.max.real, value
        assert value.imag >= self.min.imag, value
        assert value.imag <= self.max.imag, value
        #value = round (value.real, self.digits) + round (value.imag, self.digits) * 1j
        value = cround (value, self.digits)
        self.value = value
        self.inUpdate = True
        self.spinR.set_value (value.real)
        self.spinI.set_value (value.imag)
        self.inUpdate = False

    def onUpdate (self, handler):
        self.handlers.append (handler)

class PolEntry(gtk.Table):
    @staticmethod
    def __withTooltip (w, str):
        w.set_tooltip_text (str)
        return w

    def __init__ (self):
        gtk.Table.__init__ (self, 7, 6)

        self.handlers = []
        
        tip = "Normalized Jones vector with amplitude 1"
        self.attach (self.__withTooltip (gtk.Label ("Jones vector"), tip), 0, 2, 0, 1)

        tip = "X component of electric incident field"
        self.attach (self.__withTooltip (gtk.Label ("X"), tip), 0, 1, 1, 3)
        self.xEntry = self.__withTooltip (ComplexEntry (-1-1j, 1+1j), tip)
        self.xEntry.onUpdate (self.updateFromXY)
        self.xEntry.setValue (1)
        self.attach (self.xEntry, 1, 2, 1, 3)
        self.oldX = None
        self.oldY = None

        tip = "Y component of electric incident field"
        self.attach (self.__withTooltip (gtk.Label ("Y"), tip), 0, 1, 3, 5)
        self.yEntry = self.__withTooltip (ComplexEntry (-1-1j, 1+1j), tip)
        self.yEntry.onUpdate (self.updateFromXY)
        self.yEntry.setValue (0)
        self.attach (self.yEntry, 1, 2, 3, 5)

        digits = 2
        step = 0.05
        self.digits = digits

        tip = "Normalized fully polarized Stokes vector with intensity 1"
        self.attach (self.__withTooltip (gtk.Label ("Stokes vector"), tip), 2, 4, 0, 1)

        tip = "Intensity of the incident light (fixed to 1)"
        self.attach (self.__withTooltip (gtk.Label ("I"), tip), 2, 3, 1, 2)
        self.attach (self.__withTooltip (gtk.Label ("1.00"), tip), 3, 4, 1, 2)
        #tip = "Intensity of the horizontally polarized incident light - intensity of the vertically polarized incident light"
        tip = "Intensity of the horizontally / vertically polarized incident light"
        self.attach (self.__withTooltip (gtk.Label ("Q"), tip), 2, 3, 2, 3)
        self.spinQ = self.__withTooltip (gtk.SpinButton (digits=digits), tip)
        self.spinQ.set_range (-1.0, 1.0)
        self.spinQ.set_increments (step, step)
        self.spinQ.connect ("output", self.updateFromQUV)
        self.attach (self.spinQ, 3, 4, 2, 3)
        tip = "Intensity of the diagonally polarized incident light"
        self.attach (self.__withTooltip (gtk.Label ("U"), tip), 2, 3, 3, 4)
        self.spinU = self.__withTooltip (gtk.SpinButton (digits=digits), tip)
        self.spinU.set_range (-1.0, 1.0)
        self.spinU.set_increments (step, step)
        self.spinU.connect ("output", self.updateFromQUV)
        self.attach (self.spinU, 3, 4, 3, 4)
        tip = "Intensity of the circularly polarized incident light"
        self.attach (self.__withTooltip (gtk.Label ("V"), tip), 2, 3, 4, 5)
        self.spinV = self.__withTooltip (gtk.SpinButton (digits=digits), tip)
        self.spinV.set_range (-1.0, 1.0)
        self.spinV.set_increments (step, step)
        self.spinV.connect ("output", self.updateFromQUV)
        self.attach (self.spinV, 3, 4, 4, 5)
        self.oldQ = None
        self.oldU = None
        self.oldV = None

        #self.attach (gtk.Label ("Polarization ellipse"), 4, 7, 0, 1)
        self.attach (gtk.Label ("Polarization ellipse"), 4, 6, 0, 1)

        tip = "Length of the semi-major axis of the polarization ellipse"
        self.attach (self.__withTooltip (gtk.Label ("A"), tip), 4, 5, 1, 2)
        self.spinA = self.__withTooltip (gtk.SpinButton (digits=digits), tip)
        self.spinA.set_range (np.sqrt (0.5), 1.0)
        self.spinA.set_increments (step / 5, step / 5)
        self.spinA.connect ("output", self.updateFromA)
        self.attach (self.spinA, 5, 6, 1, 2)
        tip = "Length of the semi-minor axis of the polarization ellipse"
        self.attach (self.__withTooltip (gtk.Label ("B"), tip), 4, 5, 2, 3)
        self.spinB = self.__withTooltip (gtk.SpinButton (digits=digits), tip)
        self.spinB.set_range (0, np.sqrt (0.5))
        self.spinB.set_increments (step, step)
        self.spinB.connect ("output", self.updateFromB)
        self.attach (self.spinB, 5, 6, 2, 3)
        tip = "Orientation of the polarization ellipse"
        self.attach (self.__withTooltip (gtk.Label ("theta"), tip), 4, 5, 3, 4)
        self.spinT = self.__withTooltip (gtk.SpinButton (digits=digits), tip)
        #self.spinT.set_range (-np.pi/2, np.pi/2)
        #self.spinT.set_increments (step, step)
        self.spinT.set_range (-90, 90)
        self.spinT.set_increments (5, 5)
        self.spinT.set_wrap (True)
        self.spinT.connect ("output", self.updateFromTH)
        self.attach (self.spinT, 5, 6, 3, 4)
        tip = "Sense of rotation of the polarization ellipse"
        self.attach (self.__withTooltip (gtk.Label ("h"), tip), 4, 5, 4, 5)
        self.spinH = self.__withTooltip (gtk.SpinButton (digits=0), tip)
        self.spinH.set_range (-1.0, 1.0)
        self.spinH.set_increments (2.0, 2.0)
        self.spinH.connect ("output", self.updateFromTH)
        self.attach (self.spinH, 5, 6, 4, 5)
        self.oldA = None
        self.oldB = None
        self.oldT = None
        self.oldH = None

        self.labelRes = gtk.Label ()
        font_desc = pango.FontDescription('monospace')
        self.labelRes.modify_font (font_desc)
        self.labelRes.show ()
        self.attach (self.labelRes, 0, 5, 5, 6)

        tip = None
        #tip = "Polarization ellipse"
        self.figure = Figure ()
        self.axis = self.figure.add_subplot (111)
        self.canvas = self.__withTooltip (FigureCanvas (self.figure), tip)
        self.canvas.unset_flags (gtk.CAN_FOCUS)
        self.canvas.set_size_request (100, 100)
        #self.attach (self.canvas, 6, 7, 1, 6)
        self.attach (self.canvas, 6, 7, 0, 6)

        self.setValue ((1+0j, 0j))

        self.show_all ()

    def updateFromXY (self, *args):
        x = self.xEntry.value
        y = self.yEntry.value
        if x == self.oldX and y == self.oldY:
            return
        self.oldX = x
        self.oldY = y
        if self.inUpdate:
            return
        len = np.sqrt (np.abs (x ** 2) + np.abs (y ** 2))
        if len == 0:
            x = 1+0j
            y = 0j
        else:
            x /= len
            y /= len
        value = (x, y)
        self.value = value
        self.update (False, True, True, True, True)
        #print "UP"
        for handler in self.handlers:
            handler (value)

    def updateFromQUV (self, *args):
        Q = self.spinQ.get_value ()
        U = self.spinU.get_value ()
        V = self.spinV.get_value ()
        if Q == self.oldQ and U == self.oldU and V == self.oldV:
            return
        self.oldQ = Q
        self.oldU = U
        self.oldV = V
        if self.inUpdate:
            return
        Q = round (self.spinQ.get_value (), self.digits)
        U = round (self.spinU.get_value (), self.digits)
        V = round (self.spinV.get_value (), self.digits)
        Ip = np.sqrt (Q*Q + U*U + V*V)
        I = 1.0
        if Ip > 0:
            Q /= Ip
            U /= Ip
            V /= Ip
        else:
            Q = 1.0
            U = 0.0
            V = 0.0
        # http://www.utdallas.edu/~cantrell/ee6317/Lectures/polarization.pdf "FROM STOKES VECTOR TO JONES VECTOR"
        x = 0.5 * (I + Q)
        if (I + Q) * x > 0:
            #y = (U + V * 1j) / (I + Q) * x
            y = (U - V * 1j) / (I + Q) * x
        else: # 1/2 * (I + Q)² = 0 => Q = -1, x = V = U = 0 => |y| = 1, we set y = 1
            y = 1
        #print x, y, I, Q, U, V, I+Q, U-V*1j
        len = np.sqrt (np.abs (x ** 2) + np.abs (y ** 2))
        if len == 0:
            x = 1+0j
            y = 0j
        else:
            x /= len
            y /= len
        value = (x, y)
        self.value = value
        self.update (True, False, True, True, True)
        #print "UPQ", value, np.abs (value)
        for handler in self.handlers:
            handler (value)

    def updateFromA (self, *args):
        A = self.spinA.get_value ()
        if A == self.oldA:
            return
        self.oldA = A
        if self.inUpdate:
            return
        theta = self.spinT.get_value ()
        h = self.spinH.get_value ()
        A = round (A, self.digits)
        theta = round (theta, self.digits)
        if h < 0:
            h = -1
        elif h > 0:
            h = 1
        else:
            #h = 0
            h = 1

        I = 1.0
        Ip = 1.0
        B = np.sqrt (1 - A*A) # A²+B² = Ip
        Q = (A*A - B*B) * np.cos (2 * (theta / 180.0 * np.pi))
        U = (A*A - B*B) * np.sin (2 * (theta / 180.0 * np.pi))
        V = 2 * A * B * h
        # http://www.utdallas.edu/~cantrell/ee6317/Lectures/polarization.pdf "FROM STOKES VECTOR TO JONES VECTOR"
        x = 0.5 * (I + Q)
        if (I + Q) * x > 0:
            #y = (U + V * 1j) / (I + Q) * x
            y = (U - V * 1j) / (I + Q) * x
        else: # 1/2 * (I + Q)² = 0 => Q = -1, x = V = U = 0 => |y| = 1, we set y = 1
            y = 1
        #print x, y, I, Q, U, V, I+Q, U-V*1j
        len = np.sqrt (np.abs (x ** 2) + np.abs (y ** 2))
        if len == 0:
            x = 1+0j
            y = 0j
        else:
            x /= len
            y /= len
        value = (x, y)
        self.value = value
        self.update (True, True, False, True, False)
        #print "UPQ", value, np.abs (value)
        for handler in self.handlers:
            handler (value)

    def updateFromB (self, *args):
        B = self.spinB.get_value ()
        if B == self.oldB:
            return
        self.oldB = B
        if self.inUpdate:
            return
        theta = self.spinT.get_value ()
        h = self.spinH.get_value ()
        B = round (B, self.digits)
        theta = round (theta, self.digits)
        if h < 0:
            h = -1
        elif h > 0:
            h = 1
        else:
            #h = 0
            h = 1

        I = 1.0
        Ip = 1.0
        A = np.sqrt (1 - B*B) # A²+B² = Ip
        Q = (A*A - B*B) * np.cos (2 * (theta / 180.0 * np.pi))
        U = (A*A - B*B) * np.sin (2 * (theta / 180.0 * np.pi))
        V = 2 * A * B * h
        # http://www.utdallas.edu/~cantrell/ee6317/Lectures/polarization.pdf "FROM STOKES VECTOR TO JONES VECTOR"
        x = 0.5 * (I + Q)
        if (I + Q) * x > 0:
            #y = (U + V * 1j) / (I + Q) * x
            y = (U - V * 1j) / (I + Q) * x
        else: # 1/2 * (I + Q)² = 0 => Q = -1, x = V = U = 0 => |y| = 1, we set y = 1
            y = 1
        #print x, y, I, Q, U, V, I+Q, U-V*1j
        len = np.sqrt (np.abs (x ** 2) + np.abs (y ** 2))
        if len == 0:
            x = 1+0j
            y = 0j
        else:
            x /= len
            y /= len
        value = (x, y)
        self.value = value
        self.update (True, True, True, False, False)
        #print "UPQ", value, np.abs (value)
        for handler in self.handlers:
            handler (value)

    def updateFromTH (self, *args):
        theta = self.spinT.get_value ()
        h = self.spinH.get_value ()
        if theta == self.oldT and h == self.oldH:
            return
        self.oldT = theta
        self.oldH = h
        if self.inUpdate:
            return
        A = self.spinA.get_value ()
        B = self.spinB.get_value ()
        theta = round (theta, self.digits)
        if h < 0:
            h = -1
        elif h > 0:
            h = 1
        else:
            #h = 0
            h = 1

        I = 1.0
        Ip = 1.0
        ABlen = np.sqrt (A*A + B*B)
        A /= ABlen
        B /= ABlen
        Q = (A*A - B*B) * np.cos (2 * (theta / 180.0 * np.pi))
        U = (A*A - B*B) * np.sin (2 * (theta / 180.0 * np.pi))
        V = 2 * A * B * h
        # http://www.utdallas.edu/~cantrell/ee6317/Lectures/polarization.pdf "FROM STOKES VECTOR TO JONES VECTOR"
        x = 0.5 * (I + Q)
        if (I + Q) * x > 0:
            #y = (U + V * 1j) / (I + Q) * x
            y = (U - V * 1j) / (I + Q) * x
        else: # 1/2 * (I + Q)² = 0 => Q = -1, x = V = U = 0 => |y| = 1, we set y = 1
            y = 1
        #print x, y, I, Q, U, V, I+Q, U-V*1j
        len = np.sqrt (np.abs (x ** 2) + np.abs (y ** 2))
        if len == 0:
            x = 1+0j
            y = 0j
        else:
            x /= len
            y /= len
        value = (x, y)
        self.value = value
        self.update (True, True, False, False, False)
        #print "UPQ", value, np.abs (value)
        for handler in self.handlers:
            handler (value)

    def update (self, updateXY, updateQUV, updateA, updateB, updateTH):
        self.inUpdate = True

        x, y = self.value
        ffmt = "{0:+.3f}"
        cfmt = "{0.real:+.3f}{0.imag:+.3f}i"
        s = "X = " + cfmt.format (x) + ", Y = " + cfmt.format (y)

        if updateXY:
            self.xEntry.setValue (x)
            self.yEntry.setValue (y)

        I = np.abs (x ** 2) + np.abs (y ** 2)
        Q = np.abs (x ** 2) - np.abs (y ** 2)
        U = 2 * (x * y.conjugate ()).real
        V = 2 * (x * y.conjugate ()).imag
        Ip = np.sqrt (Q*Q + U*U + V*V)
        sfmt = "(I = {0:+.3f}, Q = {1:+.3f}, U = {2:+.3f}, V = {3:+.3f})"
        stokes = sfmt.format (I, Q, U, V)

        if updateQUV:
            self.spinQ.set_value (round (Q, self.digits))
            self.spinU.set_value (round (U, self.digits))
            self.spinV.set_value (round (V, self.digits))

        L = Q + U * 1j
        A = np.sqrt (0.5 * (Ip + np.abs (L)))
        B = np.sqrt (0.5 * (Ip - np.abs (L)))
        if B != B:
            B = 0
        theta = 0.5 * np.angle (L)
        h = np.sign (V)

        if updateA:
            self.spinA.set_value (round (A, self.digits))
        if updateB:
            self.spinB.set_value (round (B, self.digits))
        if updateTH:
            self.spinT.set_value (round (theta / np.pi * 180, self.digits))
            if h == 0:
                self.spinH.set_value (1.0)
            else:
                self.spinH.set_value (h)

        afmt = "(A = {0:+.3f}, B = {1:+.3f}, theta = {2:+.3f}, h = {3:+1.0f})"
        angles = afmt.format (A, B, theta, h)

        self.labelRes.set_label (s + "\n" + stokes + "\n" + angles)

        self.inUpdate = False

        self.axis.clear ()
        self.axis.set_xbound (-1.2, 1.2)
        self.axis.set_ybound (-1.2, 1.2)
        self.axis.set_aspect ("equal")
        self.axis.grid ()
        self.axis.add_artist (matplotlib.patches.Ellipse (xy=(0, 0), width=2*A, height=2*B, angle=(theta/np.pi*180), fill=False))
        if (h > 0):
            self.axis.annotate ("+", (0, 0), va="center", ha="center")
        elif (h < 0):
            self.axis.annotate ("-", (0, 0), va="center", ha="center", size="xx-large")
        self.canvas.draw ()

    def setValue (self, value):
        x, y = value
        #x = cround (x, 2)
        #y = cround (y, 2)
        self.value = (x, y)
        self.update (True, True, True, True, True)

    def onUpdate (self, handler):
        self.handlers.append (handler)
