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
metricList = []
for metricType in typeCursor:
 #   print 'F:'+str(metricType[0])+":"+metricType[1]
    typeList.insert(metricType[0], [metricType[1], metricType[2], metricType[3], metricType[4]])
#    print typeList[0][0]
    metricCursor.execute("SELECT id, name, mean, std FROM metric where metric_type_id = "+str(metricType[0]))
    for metricRow in metricCursor:
        #print metricRow
        #       metricValueCursor.execute("SELECT time, value from metric_value where metric_id = "+str(metricRow[0]))
        #        print metricValueCursor.fetchall()
        metricList.insert(metricRow[0], [metricRow[1], metricRow[2], metricRow[3]])
import time
while(1):
    time.sleep(1)
    metricValueCursor.execute("select time,value from metric_value where metric_id = 5 order by time")
#    print "set yrange ["+str(typeList[0][1])+":"+str(typeList[0][2])+"]"
    print "plot '-' title '"+metricList[5][0]+"' with lines"
    for row in metricValueCursor:
        print row[1]
    print "e"
    print "pause 1"
