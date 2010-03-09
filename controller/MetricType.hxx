// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.
#ifndef METRICTYPE_H
#define METRICTYPE_H 1

namespace systemtap
{
  class MetricType
  {
  public:
    MetricType() {};
    void setName(char* name);
    char* getName();
    void setMin(double min);
    double getMin();
    bool isMinSet();
    void setMax(double max);
    double getMax();
    bool isMaxSet();
    void setDefault(double value);
    double getDefault();
    bool isDefaultSet();
  };
}
#endif
