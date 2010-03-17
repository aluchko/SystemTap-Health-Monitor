#!/usr/bin/python
# systemtap health monitor
# Copyright (C) 2010 Aaron Luchko
#
# This file is part of systemtap, and is free software.  You can
# redistribute it and/or modify it under the terms of the GNU General
# Public License (GPL); either version 2, or (at your option) any
# later version.

from pysqlite2 import dbapi2 as sqlite
connection = sqlite.connect('/tmp/test.db')
typeCursor = connection.cursor()
metricCursor = connection.cursor()
metricValueCursor = connection.cursor()
typeCursor = connection.cursor()
typeCursor.execute("SELECT id,name,min,max,def FROM metric_type");
#print typeCursor.fetchall()
typeList = []
metricList = range(1,100) # ugly temp hack
for metricType in typeCursor:
 #   print 'F:'+str(metricType[0])+":"+metricType[1]
    typeList.insert(metricType[0], [metricType[1], metricType[2], metricType[3], metricType[4]])
#    print typeList[0][0]
    metricCursor.execute("SELECT id, name, mean, num_samples, m2 FROM metric where metric_type_id = "+str(metricType[0]))
    for metricRow in metricCursor:
        metricList[metricRow[0]] = [metricRow[1], metricRow[2], metricRow[3], metricRow[4]]

import time
import subprocess
from math import sqrt
p = subprocess.Popen("gnuplot",stdin=subprocess.PIPE,stdout=subprocess.PIPE)
while(1): # make sure we only look at graphs with > 2 samples
    time.sleep(1)
    metric = metricList[5]
        mean = metric[1]
    std = sqrt(metric[3] / (metric[2] - 1))
    metricValueCursor.execute("select time,value from metric_value where metric_id = 5 order by time")
#    p.stdin.write("set yrange ["+str(typeList[0][1])+":"+str(typeList[0][2])+"]\n")
    p.stdin.write("plot '-' title '"+metric[0]+"' with lines, '-' title '95 CI upper bound' with lines, '-' title '95 CI, lower bound' with lines\n")
    end = 0
    for row in metricValueCursor:
        p.stdin.write(str(row[1])+"\n")
        end = end+1
    p.stdin.write("e\n")
    # dumb way to get the number of rows
    for i in range(1,end):
        p.stdin.write(str(mean + std * 2)+"\n")
    p.stdin.write("e\n")
    # might fall off the bottom of the graph
    for i in range(1,end):
        p.stdin.write(str(mean - std * 2)+"\n")
    p.stdin.write("e\n")
