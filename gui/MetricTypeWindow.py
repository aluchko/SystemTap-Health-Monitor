#!/usr/bin/env python
# systemtap health monitor
# Copyright (C) 2010 Aaron Luchko
#
# This file is part of systemtap, and is free software.  You can
# redistribute it and/or modify it under the terms of the GNU General
# Public License (GPL); either version 2, or (at your option) any
# later version.

import pygtk, gtk
pygtk.require('2.0')
import sys, string
import Metrics
import time
import threading
from MetricGraph import MetricGraph
import gobject
class MetricRowElement:
# This class is a holder for the widgets associated with a single metric row we don't add this object to the container itself since we add the widgets separately so the grid works
    def __init__(self, metricTypeWindow, table, metric):
        self.table = table
        self.metric = metric
        self.lastUpdate = time.time()
        self.mtw = metricTypeWindow
        self.graphing = False

    def attach(self, currentRow):
        metric = self.metric
        table = self.table
        self.metricNameLabel = gtk.Label(metric.name)
        table.attach(self.metricNameLabel, 0, 1, currentRow, currentRow+1)
    #    self.metricNameLabel.show()

        self.metricValueLabel = gtk.Label("45")
        table.attach(self.metricValueLabel, 1, 2, currentRow, currentRow+1)
   #     self.metricValueLabel.show()

        self.metricMeanLabel = gtk.Label("%.2f" % metric.mean)
        self.table.attach(self.metricMeanLabel, 2, 3, currentRow, currentRow+1)
  #      self.metricMeanLabel.show()

        self.metricStdLabel = gtk.Label("%.2f" % metric.std)
        self.table.attach(self.metricStdLabel, 3, 4, currentRow, currentRow+1)
 #       self.metricStdLabel.show()

        self.metricButton = gtk.Button("Graph")
        self.table.attach(self.metricButton, 4, 5, currentRow, currentRow+1)
#        self.metricButton.show()
        self.lastValue = 45
        self.metricButton.connect("clicked", self.mtw.metric_button_click, self)

    def showAll(self):
        self.metricNameLabel.show()
        self.metricValueLabel.show()
        self.metricMeanLabel.show()
        self.metricStdLabel.show()
        self.metricButton.show()


    def updateValue(self, metricValue):
        self.metricValueLabel.set_text(str(metricValue))
#        self.metricValueLabel.show()
        self.lastUpdate = time.time()
        self.lastValue = metricValue
        

    # returns true if we've purged ourselves from the list
    def checkPurge(self):
        # see if we've been too long without updating
        if not self.graphing and self.lastUpdate + self.metric.metricType.timeout < time.time():
            # remove old elements from the table
            table = self.table
            table.remove(self.metricNameLabel)
            table.remove(self.metricValueLabel)
            table.remove(self.metricMeanLabel)
            table.remove(self.metricStdLabel)
            table.remove(self.metricButton)
            self.showAll()
            return True
        return False

# Simple thread that checks all the metrics every second to see if they're stale and should be removed
class MetricPurgeTimer(threading.Thread):
    def __init__(self, metricTypeWindow):
        threading.Thread.__init__(self)
        self.metricTypeWindow = metricTypeWindow
        self.quit = False
        self.stopthread = threading.Event()
        
    def run(self):
        while not self.stopthread.isSet():
            self.metricTypeWindow.purgeOldMetrics()
#            gobject.idle_add(self.metricTypeWindow.purgeOldMetrics)
            time.sleep(1)

    def stop(self):
        self.stopthread.set()

class MetricTypeWindow:
    def metric_button_click(self, widget, metricRowElement):
        if not metricRowElement.graphing:
            metricGraph = MetricGraph(metricRowElement.metric)
            metricRowElement.graphing = True
            metricRowElement.metricGraph = metricGraph
            metricRowElement.metricButton.set_label("Stop Graphing")
            self.threads.append(metricGraph)
            metricGraph.start()
        else:
            metricRowElement.graphing = False
            metricRowElement.metricGraph.stop()
            metricRowElement.metricButton.set_label("Graph")
            self.threads.remove(metricRowElement.metricGraph)

    def delete_event(self, widget, event, data=None):
        self.quit()
        return False

    def quit(self):
        self.appWindow.closed_mtw(self)
#        self.metricPurgeTimer.stop()
        for t in self.threads: # any graph threads
            t.stop()
        self.open = False


    def openGraph(self, metricTypeRow):
        metricGraph  = MetricGraph.MetricGraph(metricTypeRow.metric)
        self.threads.append(metricGraph)

    def __init__(self, metricType, appWindow):

        self.metricType = metricType
        self.appWindow = appWindow
        self.metricList = {}
        self.metricValueList = {}
        self.currentRow = 2
        self.threads = [ ]
        self.lock = threading.RLock()
        self.open = True

        # Create our window
        self.window = gtk.Dialog()
        self.window.connect("delete_event", self.delete_event)

        # Set the border width 
        self.window.set_border_width(0)
		
        # Set the window size.
        self.window.set_size_request(700, 400)

        # make it scrollable
        self.scrolled_window = gtk.ScrolledWindow()
        scrolled_window = self.scrolled_window
        scrolled_window.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.window.vbox.pack_start(scrolled_window, True, True, 0)

        scrolled_window.set_border_width(10)

        # table for the metrics
        self.table = gtk.Table(rows=2, columns=5, homogeneous=False)
        self.table.set_col_spacings(10)
        scrolled_window.add_with_viewport(self.table)
        scrolled_window.show()

        table = self.table
        
        # maybe make dialog title
        usageLabel = gtk.Label(metricType.name)
        table.attach(usageLabel, 0, 5, 0, 1)
        usageLabel.show()

        # column headers, make clickable/sortable
        metricNameLabel = gtk.Label("Metric Name")
        table.attach(metricNameLabel, 0, 1, 1, 2)
        metricNameLabel.show()

        metricValueLabel = gtk.Label("Current Value")
        table.attach(metricValueLabel, 1, 2, 1, 2)
        metricValueLabel.show()

        metricMeanLabel = gtk.Label("Long Term Mean Value")
        table.attach(metricMeanLabel, 2, 3, 1, 2)
        metricMeanLabel.show()

        metricStdLabel = gtk.Label("Standard Deviation")
        table.attach(metricStdLabel, 3, 4, 1, 2)
        metricStdLabel.show()

        self.table.show()
        self.window.show()

        # add a timer to purge old metrics from the table
#        self.metricPurgeTimer = MetricPurgeTimer(self)
 #       self.metricPurgeTimer.start()


    def update(self):
        self.lock.acquire()
        for mre in self.metricList:
            self.metricList[mre].showAll()
        self.table.show()
        self.scrolled_window.show()
        self.window.show()
        self.window.show_all()
        self.lock.release()


    def addMetric(self, metric):
        self.lock.acquire()

        metricRowElement = MetricRowElement(self, self.table, metric)
        metricRowElement.attach(self.currentRow)
        self.metricList[metric.id] = metricRowElement
        self.currentRow = self.currentRow+1

        self.lock.release()

    def updateMetric(self, metricId, metricValue):
#        self.lock.acquire()
#        gtk.gdk.threads_enter()
        self.metricList[metricId].updateValue(metricValue)
 #       gtk.gdk.threads_leave()
#        self.lock.release()
        
    def purgeOldMetrics(self):
        self.lock.acquire()
        for metricRowElementKey in self.metricList.keys():
            if (self.metricList[metricRowElementKey].checkPurge()):
                del self.metricList[metricRowElementKey]
                self.currentRow = self.currentRow - 1
        self.table.show_all()
        self.scrolled_window.show()

        self.lock.release()
