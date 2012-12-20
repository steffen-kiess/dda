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

class SnaptoCursor:
    def __init__ (self, axis, x, y, useblit=True):
        self.handlers = []
        self.axis = axis
        self.x = x
        self.y = y
        self.useblit = useblit
        self.bg = None
        self.lx = None
        self.ly = None
        self.lastX = None
        self.lastY = None
        self.lastIsReal = None

    def onUpdate (self, handler):
        self.handlers.append (handler)

    def setY (self, data):
        self.y = data
        if self.lastIsReal is not None: # Repaint cursor on data change
            self.move (self.lastX, self.lastY, self.lastIsReal)

    def move (self, x, y, isReal):
        if x is None and self.lastIsReal != isReal:
            return
        self.lastX = x
        self.lastY = y
        self.lastIsReal = isReal

        minx, maxx = self.axis.get_xlim ()
        miny, maxy = self.axis.get_ylim ()

        if self.useblit and self.bg is None:
            self.bg = self.axis.figure.canvas.copy_from_bbox (self.axis.bbox)
        self.axis.figure.canvas.restore_region (self.bg)

        #print event, id (event.inaxes), id (self.axis), event.inaxes == self.axis
        if x is not None:
            indx = np.searchsorted (self.x, [x])[0]
            if indx == len (self.x):
                indx = len (self.x) - 1
            elif indx > 0:
                if np.abs (self.x[indx - 1] - x) < np.abs (self.x[indx] - x):
                    indx = indx - 1
            x = self.x[indx]
            if indx >= len (self.y):
                y = 0
            else:
                y = self.y[indx]
            if self.lx is not None:
                self.lx.set_data ((minx, maxx), (y, y))
                self.ly.set_data ((x, x), (miny, maxy))
            else:
                #color = 'b-' if self.useblit else 'r-'
                color = 'r-'
                self.lx, = self.axis.plot ((minx, maxx), (y, y), color)
                self.ly, = self.axis.plot ((x, x), (miny, maxy), color)

            for handler in self.handlers:
                handler (indx, x, y, isReal)
        else:
            if self.lx is not None:
                self.lx.set_data ((0, 0), (0, 0))
                self.ly.set_data ((0, 0), (0, 0))
            else:
                #color = 'b-' if self.useblit else 'r-'
                color = 'r-'
                self.lx, = self.axis.plot ((0, 0), (0, 0), color)
                self.ly, = self.axis.plot ((0, 0), (0, 0), color)

            for handler in self.handlers:
                handler (None, None, None, isReal)
    
        if self.useblit:
            self.axis.draw_artist (self.lx)
            self.axis.draw_artist (self.ly)
            self.axis.figure.canvas.blit (self.axis.bbox)
        else:
            self.axis.figure.canvas.draw ()

    def mouse_move (self, event):
        if event.inaxes == self.axis:
            self.move (event.xdata, event.ydata, True)
        else:
            self.move (None, None, True)
