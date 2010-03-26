#!/usr/bin/env python
# systemtap health monitor
# Copyright (C) 2010 Aaron Luchko
#
# This file is part of systemtap, and is free software.  You can
# redistribute it and/or modify it under the terms of the GNU General
# Public License (GPL); either version 2, or (at your option) any
# later version.

import pygtk
pygtk.require('2.0')
import gtk
import sys, string
import MetricTypeWindow, Metrics, QueryAgent
#import gobject
gtk.gdk.threads_init()
#gobject.threads_init()
 
def main():
    # And of course, our main loop.
    gtk.main()
    # Control returns here when main_quit() is called
    return 0         

if __name__ =="__main__":
    metricWindow = MetricTypeWindow.MetricTypeWindow("usage")
    for t in metricWindow.threads:
        t.start()
        
    gtk.gdk.threads_enter()
    gtk.main()
    gtk.gdk.threads_leave()

