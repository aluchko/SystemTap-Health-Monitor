// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.

#include "Metric.hxx"
#include <cstring> 
namespace systemtap
{
  char* id;
  MetricType* type;
  
  Metric::Metric(MetricType* metricType, char* metricId) 
  {
    id = new char[strlen(metricId)+1];
    std::strcpy(id,metricId);
    type = metricType;
  }

  void Metric::update(int time, double value){
  }

  double Metric::getAverage()
  {
    return 0.0;
  }

  double Metric::getStd()
  {
    return 0.0;
  }
}
