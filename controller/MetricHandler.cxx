// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.

#include "MetricHandler.hxx"
#include <cstring>
#include <tr1/unordered_map>


namespace systemtap
{

  typedef std::tr1::unordered_map<std::string, Metric*> MetricTable;
  typedef std::tr1::unordered_map<std::string, MetricTable*> MetricTableTable; // maybe just use map
  typedef std::tr1::unordered_map<std::string, MetricType*> MetricTypeTable; // maybe just use map
  MetricTypeTable typeTable;
  MetricTableTable tableTable;

  void MetricHandler::addMetricType(MetricType* metricType) 
  {
    std::cout << "Add metricType :" << metricType->getName() << ":" << std::endl;
    tableTable[metricType->getName()] = new MetricTable;
    typeTable[metricType->getName()] = metricType;
}

  void MetricHandler::updateMetric(char* metricTypeName, char* metricId, int time, double value)
  {
    std::cout << "Update Metric find " << metricTypeName << std::endl;

    MetricTable* metrics = tableTable[metricTypeName];
    
    Metric* metric = (*metrics)[metricId];
    if (metric == 0)
      {
	metric = new Metric(typeTable[metricTypeName], metricId);
	(*metrics)[metricId] = metric;
      }
    metric->update(time, value);
    }
}
