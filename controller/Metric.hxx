// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.
#ifndef METRIC_H
#define METRIC_H 1

#include "Message.hxx"

// A Metric contains all the information for a single metric reported by a monitor.
namespace systemtap
{
class Metric
{
public:
  Metric() {}
  // update the Metric with the given message
  void update(Message message);
  // the average value of the metric
  double getAverage();
  // the standard deviation
  double getStd();
};
}
#endif
