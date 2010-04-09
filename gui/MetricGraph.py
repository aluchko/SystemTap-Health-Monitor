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

class MetricGraph(threading.Thread):
    def __init__ (self, metric):
        threading.Thread.__init__(self)
        self.metric = metric
        self.quit = False

        self.p = subprocess.Popen("gnuplot",stdin=subprocess.PIPE,stdout=subprocess.PIPE)
        self.stopthread = threading.Event()
        self.setup = False

    def updateGraph(self):
        print "GRAPH"
        if (not self.setup): # create in this thread to be safe
            self.connection = MySQLdb.connect(host="localhost", user="health", passwd="password", db="health")
            self.setup = True
        metricValueCursor = self.connection.cursor()

        # make sure we only look at graphs with > 2 samples
        GAP = "100000000" # last 100 seconds
        metricValueCursor.execute("select time,value from metric_value where metric_id = " + str(self.metric.id) + " and time > (select max(time) - " +GAP+" from metric_value) order by time desc")

        self.p.stdin.write("plot '-' title '"+self.metric.name+"' with lines, '-' title '95 CI upper bound' with lines, '-' title '95 CI, lower bound' with lines\n")
        end = 0
        for row in metricValueCursor:
            self.p.stdin.write(str(row[1])+"\n")
            end = end+1
        self.p.stdin.write("e\n")
            
        # dumb way to get the number of rows
        for i in range(1,end):
            self.p.stdin.write(str(self.metric.mean + self.metric.std * 2)+"\n")
        self.p.stdin.write("e\n")
        # might fall off the bottom of the graph
        for i in range(1,end):
            self.p.stdin.write(str(self.metric.mean - self.metric.std * 2)+"\n")
        self.p.stdin.write("e\n")

    def run(self):
        while not self.stopthread.isSet():
            self.updateGraph()
            sleep(0.5)

    def stop(self):
        self.stopthread.set()
        self.p.terminate()
