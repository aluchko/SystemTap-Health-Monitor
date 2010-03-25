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
import QueryAgent
import time
import threading

class MetricRowElement:
# This class is a holder for the widgets associated with a single metric row we don't add this object to the container itself since we add the widgets separately so the grid works
    def __init__(self, table, metric):
        self.table = table
        self.metric = metric
        self.lastUpdate = time.time()

    def attach(self, currentRow):
        metric = self.metric
        table = self.table
        self.metricNameLabel = gtk.Label(metric.name)
        table.attach(self.metricNameLabel, 0, 1, currentRow, currentRow+1)
        self.metricNameLabel.show()

        self.metricValueLabel = gtk.Label("45")
        table.attach(self.metricValueLabel, 1, 2, currentRow, currentRow+1)
        self.metricValueLabel.show()

        self.metricMeanLabel = gtk.Label(metric.mean)
        self.table.attach(self.metricMeanLabel, 2, 3, currentRow, currentRow+1)
        self.metricMeanLabel.show()

        self.metricStdLabel = gtk.Label(metric.std)
        self.table.attach(self.metricStdLabel, 3, 4, currentRow, currentRow+1)
        self.metricStdLabel.show()

        self.metricButton = gtk.Button("Graph")
        self.table.attach(self.metricButton, 4, 5, currentRow, currentRow+1)
        self.metricButton.show()
        self.lastValue = 45

    def updateValue(self, metricValue):
        print "update "+self.metric.name
        self.metricValueLabel.set_text(str(metricValue))
        self.metricValueLabel.show()
        if (self.lastValue != metricValue):
            self.lastUpdate = time.time()
            self.lastValue = metricValue
        

    # returns true if we've purged ourselves from the list
    def checkPurge(self):
        # see if we've been too long without updating
        if (self.lastUpdate + self.metric.metricType.timeout < time.time()):
            # remove out elements from the table
            print "REMOVE " +self.metric.name
            table = self.table
            table.remove(self.metricNameLabel)
            table.remove(self.metricValueLabel)
            table.remove(self.metricMeanLabel)
            table.remove(self.metricStdLabel)
            table.remove(self.metricButton)
            return True
        return False


class MetricPurgeTimer(threading.Thread):
    def __init__(self, metricTypeWindow, metricType):
        threading.Thread.__init__(self)
        self.metricType = metricType
        self.metricTypeWindow = metricTypeWindow
        self.quit = False
        self.stopthread = threading.Event()
        
    def run(self):
        while not self.stopthread.isSet():
            self.metricTypeWindow.purgeOldMetrics(self.metricType)
            time.sleep(1)
    def stop():
        self.stopthread.set()

class MetricTypeWindow:
    def delete_event(self, widget, event, data=None):
        print "KILL"
        for t in self.threads:
            t.stop()
        
        gtk.main_quit()
        return False

    def __init__(self, which):

        # TODO: ugly hack, better way to do this, also will need to add synchronization around these vars
        self.metricTypeList = {}
        self.metricList = {}
        self.metricValueList = {}
        self.currentRow = 2

        # Create our window
        self.window = gtk.Dialog()
        self.window.connect("delete_event", self.delete_event)

        # Set the border width 
        self.window.set_border_width(0)
		
        # Set the window size.
        self.window.set_size_request(700, 400)

        scrolled_window = gtk.ScrolledWindow()
        scrolled_window.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.window.vbox.pack_start(scrolled_window, True, True, 0)

        scrolled_window.set_border_width(10)

        self.table = gtk.Table(rows=2, columns=5, homogeneous=False)
        self.table.set_col_spacings(10)
        scrolled_window.add_with_viewport(self.table)
        scrolled_window.show()

        self.table.show()

        self.window.show()
        self.threads = [ QueryAgent.QueryAgent(self)]
        self.lock = threading.RLock()


    def addMetricType(self, metricType):
        self.lock.acquire()
        gtk.gdk.threads_enter()

        print "addMetricType"
        self.metricTypeList[metricType.id] = metricType
        table = self.table
        
        usageLabel = gtk.Label(metricType.name)
        table.attach(usageLabel, 0, 5, 0, 1)
        usageLabel.show()

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
        self.metricPurgeTimer = MetricPurgeTimer(self, metricType)
        self.threads.append(self.metricPurgeTimer)
        self.metricPurgeTimer.start()

        gtk.gdk.threads_leave()
        self.lock.release()

    def addMetric(self, metric):
        self.lock.acquire()
        gtk.gdk.threads_enter()

        metricRowElement = MetricRowElement(self.table, metric)
        metricRowElement.attach(self.currentRow)
        self.metricList[metric.id] = metricRowElement
        self.currentRow = self.currentRow+1
        self.table.show()
        self.window.show()

        gtk.gdk.threads_leave()
        self.lock.release()


    def updateMetric(self, metricId, metricValue):
        self.lock.acquire()
        gtk.gdk.threads_enter()

        self.metricList[metricId].updateValue(metricValue)

        gtk.gdk.threads_leave()
        self.lock.release()
        
    def purgeOldMetrics(self, metricType):
        print "purge"
        self.lock.acquire()
        gtk.gdk.threads_enter()

        for metricRowElementKey in self.metricList.keys():
            if (self.metricList[metricRowElementKey].checkPurge()):
                del self.metricList[metricRowElementKey]
                self.currentRow = self.currentRow - 1
        self.table.show()

        gtk.gdk.threads_leave()
        self.lock.release()
