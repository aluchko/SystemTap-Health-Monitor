import MetricTypeWindow
import Metrics

from time import sleep
import threading
from pysqlite2 import dbapi2 as sqlite

# The roll of this class is to keep the MetricTypeWindow supplied with the current list of MetricTypes, Metrics, and their current values.
class QueryAgent(threading.Thread):
    def __init__ (self,metricTypeWindow):
        threading.Thread.__init__(self)
        self.setup = False
        self.mtw = metricTypeWindow
        self.quit = False
        self.stopthread = threading.Event()
        self.lastTime = 0

    def update_metrics(self):
        print "update_metrics"
        if (not self.setup): # create in this thread to be safe
            self.connection = sqlite.connect('/tmp/test.db')
            self.setup = True
            # start with the previous to last sample
            lastTimeCursor = self.connection.cursor()
            lastTimeCursor.execute("select max(time) from metric_value where time < (select max(time) from metric_value)")
            self.lastTime = lastTimeCursor.fetchone()[0]
        typeCursor = self.connection.cursor()
        #TODO: add date column to tables to filter out old rows
        typeCursor.execute("SELECT id,name,min,max,def FROM metric_type");
        for metricTypeRow in typeCursor:
            if metricTypeRow[0] not in self.mtw.metricTypeList:
                metricType = Metrics.MetricType(metricTypeRow[0], metricTypeRow[1], metricTypeRow[2], metricTypeRow[3], metricTypeRow[4])
                self.mtw.addMetricType(metricType)

        # Every metric and it's most recent value since our last check
        metricCursor = self.connection.cursor()
        metricCursor.execute("select m.id, m.metric_type_id, m.name, m.mean, m.num_samples, m.m2, mv.value from metric m, metric_value mv where m.id=mv.metric_id and m.num_samples > 1 GROUP BY m.id having mv.time > " + str(self.lastTime) + " and mv.time = (select max(time) from metric_value where metric_value.metric_id=m.id)")

        # hrm, might miss a sample
        lastTimeCursor = self.connection.cursor()
        lastTimeCursor.execute("select max(time) from metric_value")
        lastTime = lastTimeCursor.fetchone()[0]
        # lock so none other messes around with metricList
        self.mtw.lock.acquire()
        for metricRow in metricCursor:
            # The metric could either have never been added or could have been removed
            if metricRow[0] not in self.mtw.metricList:
                metric = Metrics.Metric(metricRow[0], self.mtw.metricTypeList[metricRow[1]], metricRow[2], metricRow[3], metricRow[4], metricRow[5])
                self.mtw.addMetric(metric)
            self.mtw.updateMetric(metricRow[0], metricRow[6])
        self.mtw.lock.release()

    def run(self):
        while not self.stopthread.isSet():
#            gobject.idle_add(self.update_metrics)
            print "UPDATES"
            self.update_metrics()
            sleep(2)

    def stop(self):
        self.stopthread.set()
