// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.

#include "MetricType.hxx"
#include <cstring> 
namespace systemtap
{

  MetricType::~MetricType()
  {
    delete [] typeName;
  }
  void MetricType::setId(int dbId)
  {
    id = dbId;
  }

  int MetricType::getId()
  {
    return id;
  }

  void MetricType::setName(char* metricName){
    typeName = new char[strlen(metricName)+1];
    strcpy(typeName, metricName);
  }

  void MetricType::setMin(double minimum)
  {
    min = minimum;
    minSet = true;
  }

  void MetricType::setMax(double maximum)
  {
    max = maximum;
    maxSet = true;
  }

  void MetricType::setDefault(double value)
  {
    def = value;
    defSet = true;
  }

  char* MetricType::getName() {return typeName;}

  double MetricType::getMin() {return min;}
  bool MetricType::isMinSet() {return minSet;}

  double MetricType::getMax() {return max;}
  bool MetricType::isMaxSet() {return maxSet;}

  double MetricType::getDefault() {return def;}
  bool MetricType::isDefaultSet() {return defSet;}
}
