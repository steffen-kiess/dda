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

from cursor import SnaptoCursor

import os

import numpy as np

class LinePlotter:
    def __init__ (self, axes, fields, updateInfoDel, useTheta):
        assert len (fields) > 0

        self.axes = axes
        self.fields = fields
        self.__updateInfoDel = updateInfoDel
        self.useTheta = useTheta
        self.showLegend = True

        self.__numLines = self.getNumLines ()
        self.minValue, self.maxValue = self.getOverallRange ()
        self.__xValues = self.getXValues ()

        self.axes.set_autoscale_on (False)

        if useTheta:
            self.xdescription = u"Theta (in °)"

        self.axes.get_xaxis ().set_label_text (self.xdescription)
        self.axes.get_yaxis ().set_label_text (self.ydescription)

        self.__lines = []
        self.__lines2 = []
        for i in range (0, self.__numLines):
            self.__lines.append (self.axes.plot ([], []))
            assert (len (self.__lines[i]) == 1)
            self.__lines[i] = self.__lines[i][0]
            self.__lines[i].set_label (self.getLabel (i))

            self.__lines2.append (self.axes.plot ([], []))
            assert (len (self.__lines2[i]) == 1)
            self.__lines2[i] = self.__lines2[i][0]
            l = self.getLabel (i)
            if l is not None:
                l += " (pol2)"
            self.__lines2[i].set_label (l)
        self.haveLabel = False
        for i in range (0, self.__numLines):
            self.haveLabel = self.haveLabel or (self.__lines[i].get_label () is not None)
        self.__make_legend ()

        self.axes.set_xbound (min (self.__xValues), max (self.__xValues))
        #print self.minValue, self.maxValue
        self.axes.set_ybound (self.minValue, self.maxValue)

        self.axes.grid (True)

        if self.useTheta:
            #self.cursor = SnaptoCursor (self.axes, self.__xValues, values, useblit=True) # Blitting doesn't work with resizing etc.
            self.cursor = SnaptoCursor (self.axes, self.__xValues, [], useblit=False)
            self.axes.figure.canvas.mpl_connect ('motion_notify_event', self.cursor.mouse_move)
            self.cursor.onUpdate (self.__updateInfo)
            #cursor = matplotlib.widgets.Cursor (self.axes, useblit=True, color='red', linewidth=2)
            #self.canvas.mpl_connect ('motion_notify_event', cursor.onmove)

    def updateCPos (self, angle):
        if self.useTheta:
            if angle is None:
                self.cursor.move (None, None, False)
            else:
                self.cursor.move (angle, 0, False)

    def getRawValues (self, i, pol):
        x, y = pol
        return self.fields[i].scat (x, y)

    def getXValues (self):
        assert self.useTheta
        return self.fields[0].theta

    def getLabel (self, i):
        return os.path.realpath (self.fields[i].filename)

    def getOverallRange (self):
        min, max = self.getRange (0)
        for i in range (1, self.__numLines):
            min2, max2 = self.getRange (i)
            if min2 < min:
                min = min2
            if max2 > max:
                max = max2
        return min, max

    def plot (self, pol, pol2):
        mod = False
        for i in range (0, self.__numLines):
            values = self.getValues (i, pol)
            if self.useTheta and i == 0:
                self.cursor.setY (values)
            self.__lines[i].set_data (self.__xValues, values)
            self.__lines[i].recache ()
            if pol2 is None:
                if self.__lines2[i].get_visible ():
                    self.__lines2[i].set_data ([], [])
                    self.__lines2[i].recache ()
                    self.__lines2[i].set_visible (False)
                    mod = True
            else:
                if not self.__lines2[i].get_visible ():
                    self.__lines2[i].set_visible (True)
                    mod = True
                values = self.getValues (i, pol2)
                self.__lines2[i].set_data (self.__xValues, values)
                self.__lines2[i].recache ()
        if mod:
            self.__make_legend ()
        if self.axes.get_renderer_cache () is None:
            self.axes.figure.canvas.draw ()
        else:
            #self.axes.redraw_in_frame()
            #self.axes.figure.canvas.draw_event (self.axes.get_renderer_cache ())
            #self.axes.figure.canvas.flush_events ()
            self.axes.figure.canvas.draw ()

    def __make_legend (self):
        if self.showLegend:
            if self.haveLabel:
                h, l = self.axes.get_legend_handles_labels ()
                h2 = []
                l2 = []
                for line, label in map (None, h, l):
                    if line.get_visible ():
                        h2 += [line]
                        l2 += [label]
                self.axes.legend (h2, l2)
        else:
            self.axes.legend_ = None

    def __updateInfo (self, index, angle, value, isReal):
        self.__updateInfoDel (index, angle, value, isReal)

    def getNumLines (self):
        return len (self.fields)

    def setLegend (self, show):
        if self.showLegend != show:
            self.showLegend = show
            self.__make_legend ()
            self.axes.figure.canvas.draw ()

class AmplitudePlotter(LinePlotter):
    description = "Amplitude"
    ydescription = "Amplitude"

    def __init__ (self, axes, fields, updateInfoDel, updatePosDel):
        LinePlotter.__init__ (self, axes, fields, self.updateInfo, True)

        self.__updateInfoDel = updateInfoDel
        self.__updatePosDel = updatePosDel
    
    def getValues (self, i, pol):
        return self.getRawValues (i, pol)

    def getRange (self, i):
        return 0, np.max (self.fields[i].ampl)

    def updateInfo (self, index, angle, value, isReal):
        if index is None:
            #self.navToolbar.set_message ("")
            if isReal:
                self.__updatePosDel (None)
            self.__updateInfoDel ("")
        else:
            #self.navToolbar.set_message ("theta = {0:+08.3f}°, ampl = {1:+012.5f}".format (angle, value))
            if isReal:
                self.__updatePosDel (angle)
            self.__updateInfoDel ("theta = {0:+08.3f}°, ampl = {1:+012.5f}".format (angle, value))
        #print (data[index])
        

class FFTPlotter(LinePlotter):
    description = "FFT"
    xdescription = "Frequency"
    ydescription = "Amplitude"

    def __init__ (self, axes, fields, updateInfoDel, updatePosDel):
        self.valueCount = len (fields[0].theta) / 2 + 1

        LinePlotter.__init__ (self, axes, fields, None, False)

    def getValues (self, i, pol):
        values = self.getRawValues (i, pol)
        valuesFft = np.abs (np.fft.fft (values))
        valuesFft = valuesFft[0:len(valuesFft)/2+1] # throw away mirrored values
        return valuesFft

    def getXValues (self):
        return range (0, self.valueCount)

    def getRange (self, i):
        return 0, np.max (np.abs (np.fft.fft (self.fields[i].ampl)))


class RelErrorPlotter(LinePlotter):
    description = "Relative error"
    ydescription = "Relative error"

    def __init__ (self, axes, fields, updateInfoDel, updatePosDel):
        assert len (fields) == 2
        LinePlotter.__init__ (self, axes, fields, self.updateInfo, True)

        self.__updateInfoDel = updateInfoDel
        self.__updatePosDel = updatePosDel

    def getNumLines (self):
        return 1

    def getLabel (self, i):
        assert i == 0
        return None

    def getValues (self, i, pol):
        assert i ==  0
        values = self.getRawValues (0, pol)
        valuesExp = self.getRawValues (1, pol)
        return (values - valuesExp) / np.abs (valuesExp)

    def getRange (self, i):
        assert i ==  0
        return -1, 1

    def updateInfo (self, index, angle, value, isReal):
        if index is None:
            #self.navToolbar.set_message ("")
            if isReal:
                self.__updatePosDel (None)
            self.__updateInfoDel ("")
        else:
            #self.navToolbar.set_message ("theta = {0:+08.3f}°, ampl = {1:+012.5f}".format (angle, value))
            if isReal:
                self.__updatePosDel (angle)
            self.__updateInfoDel ("theta = {0:+08.3f}°, rel error = {1:+012.5f}".format (angle, value))
        #print (data[index])
