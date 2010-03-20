import MetricTypeWindow
import Metrics
import gobject

from time import sleep
from threading import Thread
from pysqlite2 import dbapi2 as sqlite

# The roll of this class is to keep the MetricTypeWindow supplied with the current list of MetricTypes, Metrics, and their current values.
class QueryAgent(Thread):
    def __init__ (self,metricTypeWindow):
        Thread.__init__(self)
        self.setup = False
        self.mtw = metricTypeWindow
        self.quit = False

    def update_metrics(self):
        print "update_metrics"
        if (not self.setup):
            self.connection = sqlite.connect('/tmp/test.db')
            self.setup = True
        typeCursor = self.connection.cursor()
        #TODO: add date column to tables to filter out old rows
        typeCursor.execute("SELECT id,name,min,max,def FROM metric_type");
        for metricTypeRow in typeCursor:
            if (not metricTypeRow[0] in self.mtw.metricTypeList):
                metricType = Metrics.MetricType(metricTypeRow[0], metricTypeRow[1], metricTypeRow[2], metricTypeRow[3], metricTypeRow[4])
                self.mtw.addMetricType(metricType)

        # Every metric and it's most recent value, not this isn't quite right as the time should be cutoff to sometime in the last cycle or so, unfortunately we lose precision in time since microseconds are too many digits to store in the DB
        metricCursor = self.connection.cursor()
        metricCursor.execute("select m.id, m.metric_type_id, m.name, m.mean, m.num_samples, m.m2, mv.value from metric m, metric_value mv where m.id=mv.metric_id and m.num_samples > 1 GROUP BY m.id having mv.time = (select max(mv.time))")

        for metricRow in metricCursor:
            if (not metricRow[0] in self.mtw.metricList):
                metric = Metrics.Metric(metricRow[0], self.mtw.metricTypeList[metricRow[1]], metricRow[2], metricRow[3], metricRow[4], metricRow[5])
                self.mtw.addMetric(metric)
            metric = self.mtw.metricList[metricRow[0]]
            self.mtw.updateMetric(metricRow[0], metricRow[6])

    def run(self):
        while not self.quit:
            gobject.idle_add(self.update_metrics)
            sleep(1)
