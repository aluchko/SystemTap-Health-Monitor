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
#include <mysql++.h>
#include <mysqld_error.h>
#include <qparms.h>
#include <iostream>
namespace systemtap
{

  class MetricHandler
  {
  private:
    mysqlpp::Connection *conn;
    mysqlpp::Query *insert_metrictype_stmt;
    mysqlpp::Query *find_metrictype_stmt;
    mysqlpp::Query *insert_metric_stmt;
    mysqlpp::Query *update_metric_stmt;
    mysqlpp::Query *find_metric_stmt;
    mysqlpp::Query *insert_metricvalue_stmt;
    std::string str(double value);

  public:
    MetricHandler(mysqlpp::Connection *connection);
    void addMetricType(MetricType* metricType);
    void updateMetric(char* metricTypeName, char* metricId, double time, double value);
    void persistUpdates();
  };
}
#endif
