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

# Show far field files
#
# Usage: show-field.py <directory or file> [...]
#
# Example: Python/show-field.py DDA/output/latest

import sys
import os
import gtk
import pango

import matplotlib
matplotlib.use('GTK')

import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
from matplotlib.figure import Figure
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas
from matplotlib.backends.backend_gtkagg import NavigationToolbar2GTKAgg as NavigationToolbar

from field_data import FieldData
from entry import PolEntry
from plotter import AmplitudePlotter, FFTPlotter, RelErrorPlotter
from plot_viewer import PlotViewer

def printfun (x):
    print x

class FieldViewer:
    def __init__ (self, paths):
        assert len (paths) > 0
        self.__fields = []
        for p in paths:
            self.__fields.append (FieldData.open (p))

        self.builder = gtk.Builder ()
        #dirname = os.path.realpath (os.path.dirname (sys.argv[0]))
        dirname = os.path.dirname (sys.argv[0])
        self.builder.add_from_file (os.path.join (dirname, "show-field.glade"))

        self.builder.connect_signals (self)
        for obj in self.builder.get_objects ():
            name = gtk.Buildable.get_name (obj)
            assert name not in self.__dict__
            self.__dict__[name] = obj

        plotters = [(AmplitudePlotter, True), (FFTPlotter, False)]
        if len (self.__fields) == 2:
            plotters.append ((RelErrorPlotter, True))
        self.plotViewer = PlotViewer (plotters, self.__fields)
        self.plotViewer.onUpdateInfo (self.updateInfo)
        self.amplBox.pack_start (self.plotViewer, True, True)
        self.plotViewer.show ()

        self.pol2CB = gtk.CheckButton ("Second polarization")
        self.plotViewer.checkboxes.pack_start (self.pol2CB, False, False)
        self.plotViewer.checkboxes.reorder_child (self.pol2CB, 0)
        self.pol2CB.connect ("toggled", self.__on_pol2cb_toggled)
        self.pol2CB.show ()

        font_desc = pango.FontDescription('monospace')
        self.infoLabel.modify_font (font_desc)

        gtk.window_set_default_icon_name (gtk.STOCK_DIALOG_INFO)

        self.statusbar_cid = self.statusbar.get_context_id ("Foo bar")
        self.statusbar.push (self.statusbar_cid, "")

        self.pol = PolEntry ()
        self.pol.onUpdate (self.plotViewer.setPol)
        self.paramHBox.pack_start (self.pol)

        self.pol2 = PolEntry ()
        self.pol2.onUpdate (self.plotViewer.setPol2)
        self.paramHBox2.pack_start (self.pol2)
        self.expander2.hide ()

    def __on_pol2cb_toggled (self, button):
        if button.get_active ():
            self.expander2.show ()
            self.plotViewer.setPol2 (self.pol2.value)
        else:
            self.expander2.hide ()
            self.plotViewer.setPol2 (None)

    def updateInfo (self, info):
        self.infoLabel.set_label (info)

    def on_window_destroy (self, widget, data=None):
        gtk.main_quit ()

    def on_window_delete_event (self, widget, event, data=None):
        return False

if len (sys.argv) <= 1:
    paths = ['.']
else:
    paths = sys.argv[1:]
FieldViewer (paths).window.show ()
gtk.main ()
