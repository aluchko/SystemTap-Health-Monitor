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
import DateTimeChooser
import gobject
class MetricRowElement:
# This class is a holder for the widgets associated with a single metric row we don't add this object to the container itself since we add the widgets separately so the grid works
    def __init__(self, metricTypeWindow, table, metric):
        self.table = table
        self.metric = metric
        self.lastUpdate = time.time()
        self.mtw = metricTypeWindow
        self.graphing = False
        self.value = 0


    def apply(self, widgets):
        metric = self.metric
        self.metricButton = widgets[4]
        if abs(self.value - self.metric.mean) > self.metric.std*2:
            widgets[0].set_markup("<b>"+self.metric.name+"</b>")
            widgets[1].set_markup("<b>"+str(self.value)+"</b>")
            widgets[2].set_markup("<b>%.2f" % self.metric.mean+"</b>")
            widgets[3].set_markup("<b>%.2f" % self.metric.std+"</b>")
        else:
            widgets[0].set_text(self.metric.name)
            widgets[1].set_text(str(self.value))
            widgets[2].set_text("%.2f" % self.metric.mean)
            widgets[3].set_text("%.2f" % self.metric.std)
        handlerId = self.metricButton.connect("clicked", self.mtw.metric_button_click, self)
        if self.graphing:
            self.metricButton.set_label("Stop Graphing")
        else:
            self.metricButton.set_label("Graph")
        return handlerId

    def updateValue(self, metricValue):
        self.lastUpdate = time.time()
        self.value = metricValue
        

    # returns true if we've purged ourselves from the list
    def checkPurge(self):
        # see if we've been too long without updating
        if not self.graphing and self.lastUpdate + self.metric.metricType.timeout < time.time():
            return True
        return False

    def __repr__(self):
        return repr((self.metric.name, self.value, self.metric.mean, self.metric.std))
    
    def __getitem__(self, index):
        values = {0: self.metric.name, 1: self.value, 2: self.metric.mean, 3: self.metric.std}
        return values[index];



from operator import itemgetter
class MetricTypeWindow:
    def metric_button_click(self, widget, metricRowElement):
        if not metricRowElement.graphing:
            metricGraph = MetricGraph(metricRowElement)
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

    def sort_button_click(self, widget, column):
        if self.column == column:
            self.reverseSort = not self.reverseSort
        self.column = column
        self.update()

    def delete_event(self, widget, event, data=None):
        self.quit()
        return False

    def quit(self):
        self.appWindow.closed_mtw(self)
        for t in self.threads: # any graph threads
            t.stop()


    def openGraph(self, metricTypeRow):
        metricGraph  = MetricGraph.MetricGraph(metricTypeRow.metric)
        self.threads.append(metricGraph)

    def __init__(self, metricType, appWindow):

        self.metricType = metricType
        self.appWindow = appWindow
        self.metricDict = {} # dictionary of all metrics stored by id
        self.metricList = {} # metricRowElements in display order
        self.widgetList = [] # none of the gtk containers let us access elements very well, so we need our own list of the widgets that we can access easily
        self.handlerList = [] # list of handler IDs for the graph buttons
        self.currentRow = 2
        self.threads = [ ]
        self.lock = threading.RLock()
        self.column = 1 # sort by current value to start
        self.reverseSort = True

        # Create our window
        self.window = gtk.Dialog()
        self.window.connect("delete_event", self.delete_event)

        # Set the border width 
        self.window.set_border_width(0)
		
        # Set the window size.
        self.window.set_size_request(900, 500)

        # make it scrollable
        self.scrolled_window = gtk.ScrolledWindow()
        scrolled_window = self.scrolled_window
        scrolled_window.set_policy(gtk.POLICY_AUTOMATIC, gtk.POLICY_AUTOMATIC)
        self.window.vbox.pack_start(scrolled_window, True, True, 0)
        
        # add and initialize just to remove in update()
        self.table = gtk.Table(rows=2, columns=5, homogeneous=True)
        self.viewport = gtk.Viewport()
        self.viewport.add(self.table)
        scrolled_window.add(self.viewport)

        scrolled_window.set_border_width(10)
        # display initially

        # maybe make dialog title
        usageLabel = gtk.Label(self.metricType.name)
        usageLabel.set_size_request(100,50)
        self.table.attach(usageLabel, 0, 5, 0, 1, yoptions=gtk.EXPAND)
        usageLabel.show()

        dtChooser = DateTimeChooser.DateTimeChooser(self)
        self.table.attach(dtChooser, 0, 5, 1, 2, yoptions=gtk.EXPAND)
        

        # column headers, make clickable/sortable
        nameButton = gtk.Button("Metric Name")
        self.table.attach(nameButton, 0, 1, 2, 3,  yoptions=gtk.EXPAND)
        nameButton.show()
        nameButton.connect("clicked", self.sort_button_click, 0)

        valueButton = gtk.Button("Current Value")
        self.table.attach(valueButton, 1, 2, 2, 3, yoptions=gtk.EXPAND)
        valueButton.show()
        valueButton.connect("clicked", self.sort_button_click, 1)

        meanButton = gtk.Button("Long Term Mean Value")
        self.table.attach(meanButton, 2, 3, 2, 3, yoptions=gtk.EXPAND)
        meanButton.show()
        meanButton.connect("clicked", self.sort_button_click, 2)

        stdButton = gtk.Button("Standard Deviation")
        self.table.attach(stdButton, 3, 4, 2, 3, yoptions=gtk.EXPAND)
        stdButton.show()
        stdButton.connect("clicked", self.sort_button_click, 3)

        self.update()

    # update the display, need to rebuild the entire table since we can't insert rows
    def update(self):
        self.lock.acquire()

        metricList = self.metricDict.values()
        sortedList = sorted(metricList, key=itemgetter(self.column), reverse=self.reverseSort)
        offset = 4;

        # we can't move widgets around the table effectively, so we'll just re-write placed widgets to their correct values

        # if we don't have enough widgets for all the metrics add more rows
        for i in range(len(self.widgetList), len(sortedList)):
            mnameLabel = gtk.Label("")
            self.table.attach(mnameLabel, 0, 1, i+offset, i+offset+1, yoptions=gtk.EXPAND)
            mcurrLabel = gtk.Label("")
            self.table.attach(mcurrLabel, 1, 2, i+offset, i+offset+1, yoptions=gtk.EXPAND)
            mmeanLabel = gtk.Label("")
            self.table.attach(mmeanLabel, 2, 3, i+offset, i+offset+1, yoptions=gtk.EXPAND)
            mstdLabel = gtk.Label("")
            self.table.attach(mstdLabel, 3, 4, i+offset, i+offset+1, yoptions=gtk.EXPAND)
            graphButton = gtk.Button("")
            self.table.attach(graphButton, 4, 5, i+offset, i+offset+1, yoptions=gtk.EXPAND)

            self.widgetList.append([mnameLabel, mcurrLabel, mmeanLabel, mstdLabel, graphButton]);
            self.handlerList.append(-1)

        widgetPopList = [];
        handlerPopList = [];
        # if we have extra widgets remove them
        for i in range(len(sortedList), len(self.widgetList)):
            for widget in self.widgetList[i]:
                self.table.remove(widget)
            widgetPopList.append(self.widgetList[i]) #remove later
            handlerPopList.append(self.handlerList[i]) #remove later
                
        # can do this better latber but don't want to worry about modifying the list as we iterate it for now
        for widgets in widgetPopList:
            self.widgetList.remove(widgets)
        for handler in handlerPopList:
            self.handlerList.remove(handler)

        for i in range(len(sortedList)):
            widgets =self.widgetList[i]
            if self.handlerList[i] != -1: # new button
                widgets[4].disconnect(self.handlerList[i]) # disconnect graph button
            self.handlerList[i] = sortedList[i].apply(self.widgetList[i]) # relevant widget row

        self.window.show_all()
        self.lock.release()

    def addMetric(self, metric):
        self.lock.acquire()

        metricRowElement = MetricRowElement(self, self.table, metric)
        self.metricDict[metric.id] = metricRowElement
        self.currentRow = self.currentRow+1

        self.lock.release()

    def updateMetric(self, metricId, metricValue):
        self.lock.acquire()
        self.metricDict[metricId].updateValue(metricValue)
        self.lock.release()
        
    def purgeOldMetrics(self):
        self.lock.acquire()
        for metricRowElementKey in self.metricDict.keys():
            if (self.metricDict[metricRowElementKey].checkPurge()):
                del self.metricDict[metricRowElementKey]
                self.currentRow = self.currentRow - 1

        self.lock.release()
