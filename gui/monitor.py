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
import AppWindow
import QueryAgent
import Metrics
from threading import Thread
gtk.gdk.threads_init()

import MySQLdb

# connection just to get the metricTypes
# connect
connection = MySQLdb.connect(host="localhost", user="health", passwd="password", db="health")

mtCursor = connection.cursor()
mtCursor.execute("SELECT id,name,min,max,def FROM metric_type")
appWindow = AppWindow.AppWindow()
for metricTypeRow in mtCursor:
    metricType = Metrics.MetricType(metricTypeRow[0], metricTypeRow[1], metricTypeRow[2], metricTypeRow[3], metricTypeRow[4])
    appWindow.addMetricType(metricType)

# have to leave it since connections can't change threads
connection.close()

queryAgent = QueryAgent.QueryAgent(appWindow)
queryAgent.start()

gtk.gdk.threads_enter()
gtk.main()
gtk.gdk.threads_leave()

