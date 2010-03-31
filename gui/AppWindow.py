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
import threading
import MetricTypeWindow
class AppWindow:

    def __init__(self):
        self.mtws = {}
        self.lock = threading.RLock()
        # Create our window
 #       gtk.gdk.threads_enter()
        self.window = gtk.Dialog()
        self.window.connect("delete_event", self.delete_event)
        self.window.connect("destroy", self.destroy)
        self.table = gtk.Table(rows=2, columns=1, homogeneous=False)
        self.window.vbox.pack_start(self.table, True, True, 0)
        self.window.show()
        self.stopthread = threading.Event()
        self.currentRow = 0
#        gtk.gdk.threads_leave()

    def delete_event(self, widget, event, data=None):
        print "APPDEL"
        self.quit()
        print "DONE"
        return False

    def destroy(self, widget, data=None):
        print "APPDEST"
        self.quit()
        gtk.main_quit()

    def quit(self):
        self.stopthread.set()
        self.lock.acquire()
        # mtw.quit() will remove items from self.mtws which would screw 
        # up the loop. so just copy the list instead
        mtwList = []
        mtwList.extend(self.mtws.values())
        for mtw in mtwList:
            mtw.quit()
        self.lock.release()
    
    def closed_mtw(self, mtw):
        self.lock.acquire()
        del self.mtws[mtw.metricType.id]
        self.lock.release()

    def metric_type_button_click(self, widget, metricType):
        if metricType.id not in self.mtws:
            self.mtws[metricType.id] = MetricTypeWindow.MetricTypeWindow(metricType, self)


    def addMetricType(self, metricType):
        print "ADD " + metricType.name
        gtk.gdk.threads_enter()
        self.lock.acquire()

        button = gtk.Button(metricType.name)
        button.connect("clicked", self.metric_type_button_click, metricType)
        self.table.attach(button,0, 1, self.currentRow, self.currentRow+1)
        self.currentRow = self.currentRow+1
        gtk.gdk.threads_leave()
        button.show()
        self.table.show()
        self.window.show()
        self.lock.release()
        print "ADD DONE " + metricType.name
