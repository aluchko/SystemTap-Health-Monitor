// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.
#ifndef METRICHANDLER_H
#define METRICHANDLER_H 1
#include "MetricType.hxx"
#include "Metric.hxx"
#include <tr1/unordered_map>
//#include "DbHandler.hxx"
#include <sstream>
#include <sqlite3.h>

namespace systemtap
{

  class MetricHandler
  {
  private:
    sqlite3* db;
    sqlite3_stmt* insert_metrictype_stmt;
    sqlite3_stmt* find_metrictype_stmt;
    sqlite3_stmt* insert_metric_stmt;
    sqlite3_stmt* update_metric_stmt;
    sqlite3_stmt* find_metric_stmt;
  public:
    MetricHandler(sqlite3* database);
    void addMetricType(MetricType* metricType);
    void updateMetric(char* metricTypeName, char* metricId, int time, double value);
    void persistUpdates();
  };
}
#endif
