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

import gtk

import matplotlib as mpl
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas
from matplotlib.backends.backend_gtkagg import NavigationToolbar2GTKAgg as NavigationToolbar
from time import time

class PlottedData:
    def __init__ (self, axes, plotter, default, updateChildren):
        self.axes = axes
        self.plotter = plotter
        self.axes.set_visible (default)
        self.checkBox = gtk.CheckButton (self.plotter.description)
        self.checkBox.set_active (default)
        self.checkBox.connect ("toggled", self.__on_toggled)
        self.checkBox.show ()
        self.updateChildren = updateChildren
        #print "PlottedData (axes = %s, plotter = %s, name = %s)" % (id (self.axes), id (self.plotter), plotter.description)

    def __on_toggled (self, button):
        self.axes.set_visible (self.checkBox.get_active ())
        self.updateChildren ()


class PlotViewer(gtk.VBox):
    def __init__ (self, plotters, fields):
        gtk.VBox.__init__ (self)

        self.figure = mpl.figure.Figure ()
        self.canvas = FigureCanvas (self.figure)
        self.canvas.unset_flags (gtk.CAN_FOCUS)
        self.canvas.set_size_request (600, 400)
        self.pack_start (self.canvas, True, True)
        self.canvas.show ()

        self.navToolbar = NavigationToolbar (self.canvas, self.window)
        #self.navToolbar.lastDir = '/tmp'
        self.pack_start (self.navToolbar, False, False)
        self.navToolbar.show ()

        self.checkboxes = gtk.HBox (len (plotters))
        self.pack_start (self.checkboxes, False, False)
        self.checkboxes.show ()

        self.pol = (1+0j, 0j)
        self.pol2 = None

        self.handlers = []

        i = 0
        self.plots = []
        for plotterClass, default in plotters:
            axes = self.figure.add_subplot (len (plotters), 1, i)
            def makeUpdateInfo (i): return lambda s: self.__updateInfo (i, s)
            def makeUpdatePos (i): return lambda s: self.__updatePos (i, s)
            plotter = plotterClass (axes, fields, makeUpdateInfo (i), makeUpdatePos (i))
            d = PlottedData (axes, plotter, default, self.__updateChildren)
            self.checkboxes.pack_start (d.checkBox, False, False)
            self.plots.append (d)
            i += 1
        self.__infos = [None] * len (self.plots)
        self.__posi = None

        self.legendBox = gtk.CheckButton ("Show legend")
        self.legendBox.set_active (True)
        self.legendBox.connect ("toggled", self.__on_legend_toggled)
        self.checkboxes.pack_start (self.legendBox, False, False)
        self.legendBox.show ()

        self.__updateChildren ()

    def __on_legend_toggled (self, button):
        for pd in self.plots:
            pd.plotter.setLegend (button.get_active ())

    def __updateChildren (self):
        count = 0
        for axes in self.figure.get_axes ():
            visible = axes.get_visible ()
            if axes.get_visible ():
                count += 1
        if count == 0:
            count = 1
        nr = 1
        for axes in self.figure.get_axes ():
            axes.change_geometry (count, 1, nr)
            if axes.get_visible ():
                if nr < count:
                    nr += 1
            else:
                axes.set_position ((0, 0, 1e-10, 1e-10)) # Hack to prevent the invisible axes from getting mouse events
        self.figure.canvas.draw ()
        self.__updateGraph ()

    def __updateGraph (self):
        for pd in self.plots:
            if pd.axes.get_visible ():
                #start = time ()
                pd.plotter.plot (self.pol, self.pol2)
                #print "Plot ", pd.plotter.description, " needed ", time () - start

    def __updateInfo (self, i, arg):
        #print i, arg
        self.__infos[i] = arg
        s = ''
        for info in self.__infos:
            if info is not None:
                if s != '':
                    s += ' '
                s += info
        for handler in self.handlers:
            handler (s)

    def __updatePos (self, i, arg):
        if arg == None and self.__posi != i:
            return
        self.__posi = i
        j = 0
        for pd in self.plots:
            if i != j:
                pd.plotter.updateCPos (arg)
            j += 1

    def onUpdateInfo (self, handler):
        self.handlers.append (handler)

    def setPol (self, value):
        oldValue = self.pol
        self.pol = value
        if value != oldValue:
            self.__updateGraph ()

    def setPol2 (self, value):
        oldValue = self.pol2
        self.pol2 = value
        if value != oldValue:
            self.__updateGraph ()
