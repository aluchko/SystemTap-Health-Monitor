#!/usr/bin/python
# systemtap health monitor
# Copyright (C) 2010 Aaron Luchko
#
# This file is part of systemtap, and is free software.  You can
# redistribute it and/or modify it under the terms of the GNU General
# Public License (GPL); either version 2, or (at your option) any
# later version.

from time import sleep
import threading 
import subprocess
import Metrics
import MySQLdb
import tempfile
import pygtk
pygtk.require('2.0')
import gtk
import datetime

class MetricGraph(threading.Thread):
    def __init__ (self, metricRowElement):
        threading.Thread.__init__(self)
        self.mre = metricRowElement
        self.metric = self.mre.metric
        
        self.p = subprocess.Popen("gnuplot",stdin=subprocess.PIPE,stdout=subprocess.PIPE)
        self.stopthread = threading.Event()
        self.setup = False
        self.imageFile = tempfile.NamedTemporaryFile()
        print self.imageFile.name

        # want to save plots as pngs
        self.p.stdin.write("set terminal png\n")
        self.window = gtk.Dialog()
        self.window.connect("delete_event", self.delete_event)
        self.window.connect("destroy", self.destroy)
        self.image = gtk.Image()
        self.window.vbox.pack_start(self.image)
        self.lock = threading.RLock()

    def updateGraph(self):
        gtk.gdk.threads_enter()
        
        if (not self.setup): # create in this thread to be safe
            self.connection = MySQLdb.connect(host="localhost", user="health", passwd="password", db="health")
            self.setup = True
            minMaxCursor = self.connection.cursor()
            minMaxCursor.execute("select min, max from metric_type where id = " + str(self.metric.metricType.id))
            print("select min, max from metric_type where id = " + str(self.metric.metricType.id))
 #           print minMaxCursor.fetchall()
            minMax = minMaxCursor.fetchone()
            self.min = minMax[0]
            self.max = minMax[1]
            print minMax

        metricValueCursor = self.connection.cursor()

        timeFmt = "%H:%M:%S";

        # gnuplot will write plot to this file
        self.p.stdin.write('set output "'+self.imageFile.name+'"\n')

        self.p.stdin.write('set xdata time\n')
        self.p.stdin.write('set timefmt "'+timeFmt+'"\n')
#        self.p.stdin.write('set format x "'+timeFmt+'"\n')

        useMin = False
        useMax = False
        if self.min is not None:
            if self.metric.mean - self.metric.std * 2 < self.min:
                useMin = True
        if self.max is not None:
            if self.metric.mean + self.metric.std * 2 > self.max:
                useMax = True
        if useMin and useMax:
            self.p.stdin.write("set yrange [" + str(self.min) + ":" + str(self.max) + "]\n")
        elif useMin:
            self.p.stdin.write("set yrange [" + str(self.min) + ":]\n")
        elif useMax:
            self.p.stdin.write("set yrange [:" + str(self.max) + "]\n")

        end = 0

        # make sure we only look at graphs with > 2 samples
        GAP = "100000000" # last 100 seconds
        metricValueCursor.execute("select time,value from metric_value where metric_id = " + str(self.metric.id) + " and time > (select max(time) - " +GAP+" from metric_value) order by time")
              
        valueList = metricValueCursor.fetchall()
        firstValue = valueList[0]
        lastValue = valueList[len(valueList)-1]
#        self.p.stdin.write('set xrange["' + datetime.datetime.fromtimestamp(firstValue[0]/1000000).strftime(timeFmt)+'":"'+datetime.datetime.fromtimestamp(lastValue[0]/1000000).strftime(timeFmt)+'"]\n')

        self.p.stdin.write("plot '-' using 1:2 title '"+self.metric.name+"' with lines")
        if not useMax:
            self.p.stdin.write(", '-' using 1:2 title '95 CI upper bound' with lines")
        if not useMin:
            self.p.stdin.write(", '-' using 1:2 title '95 CI, lower bound' with lines")
        self.p.stdin.write("\n")
        i = 0
        for row in valueList:
            dt = datetime.datetime.fromtimestamp(row[0]/1000000).strftime("%H:%M:%S")
            self.p.stdin.write(str(dt) + " " + str(row[1])+"\n")
            end = end+1
        self.p.stdin.write("e\n")
            
        # dumb way to get the number of rows
        if not useMax:
            for row in valueList:
                dt = datetime.datetime.fromtimestamp(row[0]/1000000).strftime("%H:%M:%S")
                self.p.stdin.write(dt + " " +str(self.metric.mean + self.metric.std * 2)+"\n")
            self.p.stdin.write("e\n")

        if not useMin:
            for row in valueList:
                dt = datetime.datetime.fromtimestamp(row[0]/1000000).strftime("%H:%M:%S")
                self.p.stdin.write(dt + " " +str(self.metric.mean - self.metric.std * 2)+"\n")
            self.p.stdin.write("e\n")

        sleep(0.2)
        self.image.set_from_file(self.imageFile.name)
        self.image.show()
        self.window.show()
        self.image.show()
        gtk.gdk.threads_leave()

    def run(self):
        while not self.stopthread.isSet():
            self.updateGraph()
            sleep(1)

    def stop(self):
        self.window.destroy()

    def quit(self):
        self.mre.graphing = False
        self.mre.metricButton.set_label("Graph")
        self.p.terminate()
        self.stopthread.set()
        print "QUIT GRAPH"

    def delete_event(self, widget, event, data=None):
        self.quit()
        return False

    def destroy(self, widget, data=None):
        print "destroy graph"
        self.quit()
        print "destroyed graph"
