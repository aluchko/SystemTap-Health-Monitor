#!/usr/bin/env python

import pygtk, gtk
pygtk.require('2.0')
import sys, string
import Metrics

class MetricTypeWindow:
    def delete_event(self, widget, event, data=None):
        gtk.main_quit()
        return False

    def __init__(self, which):

        # TODO: ugly hack, better way to do this, also will need to add synchronization around these vars
        self.metricTypeList = {}
        self.metricList = {}
        self.metricValueList = {}
        self.currentRow = 2

        # Create our window
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("delete_event", self.delete_event)
        self.window.set_border_width(10)
        self.window.connect("destroy", lambda _: gtk.main_quit())

        self.table = gtk.Table(rows=2, columns=5, homogeneous=False)
        self.table.set_col_spacings(10)

        self.table.show()
        self.window.show()

    def addMetricType(self, metricType):

        print "addMetricType"
        self.metricTypeList[metricType.id] = metricType
        table = self.table
        self.window.add(table)
        
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
        print "ADDED"

        
    def addMetric(self, metric):
        print "add metric"
        self.metricList[metric.id] = metric

        table = self.table
        currentRow = self.currentRow
        metricNameLabel = gtk.Label(metric.name)
        table.attach(metricNameLabel, 0, 1, currentRow, currentRow+1)
        metricNameLabel.show()

        metricValueLabel = gtk.Label("45")
        table.attach(metricValueLabel, 1, 2, currentRow, currentRow+1)
        metricValueLabel.show()

        metricMeanLabel = gtk.Label(metric.mean)
        table.attach(metricMeanLabel, 2, 3, currentRow, currentRow+1)
        metricMeanLabel.show()

        metricStdLabel = gtk.Label(metric.std)
        table.attach(metricStdLabel, 3, 4, currentRow, currentRow+1)
        metricStdLabel.show()

        metricButton = gtk.Button("Graph")
        table.attach(metricButton, 4, 5, currentRow, currentRow+1)
        metricButton.show()

        self.metricValueList[metric.id] = [currentRow, metricValueLabel]
        print metric.id
        self.currentRow = currentRow+1
        self.table.show()
        self.window.show()

    def updateMetric(self, metricId, metricValue):
        metricValueLabel = self.metricValueList[metricId][1]
        metricRow = self.metricValueList[metricId][0]
        metricValueLabel.set_text(str(metricValue))
        metricValueLabel.show()
