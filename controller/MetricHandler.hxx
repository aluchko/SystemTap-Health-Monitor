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
namespace systemtap
{
  class MetricHandler
  {
  public:
    MetricHandler() {};
    void addMetricType(MetricType* metricType);
    void updateMetric(char* metricTypeName, char* metricId, int time, double value);
  };
}
#endif
