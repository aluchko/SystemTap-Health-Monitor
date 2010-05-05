// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.

#include "Metric.hxx"
#include <cstring>
#include <cmath>
namespace systemtap
{

  Metric::Metric(MetricType* metricType, char* metricName) 
  {
    name = new char[strlen(metricName)+1];
    std::strcpy(name,metricName);
    type = metricType;
    n = 0;
    m2 = 0;
    mean = 0;
    updated = false;
  }

  Metric::~Metric()
  {
    delete [] name;
    delete [] type;
  }

  char* Metric::getName()
  {
    return name;
  }

  void Metric::setTime(double t)
  {
    time = t;
    setUpdated(true);
  }

  double Metric::getTime()
  {
    return time;
  }

  void Metric::setCurrentValue(double currentValue)
  {
    currentVal = currentValue;
    setUpdated(true);
  }

  double Metric::getCurrentValue()
  {
    return currentVal;
  }

  void Metric::setId(int dbId)
  {
    id = dbId;
    setUpdated(true);
  }

  int Metric::getId()
  {
    return id;
  }

  // Add a new sample to this metric
  void Metric::update(double time, double value){
    // formula to calculate running sum and variance
    n++;
    double delta = value - mean;
    mean = mean + delta/n;
    m2 = m2 + delta * (value - mean);
    setCurrentValue(value);
    setTime(time);
    setUpdated(true);
  }

  double Metric::getMean()
  {
    return mean;
  }

  double Metric::getStd()
  {
    if (n < 2) // better than an infinity
      return std::sqrt(m2);
    double variance = m2/(n - 1); // n-1 as we only have a sample of the metric
    return std::sqrt(variance);
  }

  void Metric::setMean(double value)
  {
    mean = value;
    setUpdated(true);
  }

  void Metric::setM2(double value)
  {
    m2 = value;
    setUpdated(true);
  }

  void Metric::setNumSamples(int value)
  {
    n = value;
    setUpdated(true);
  }

  double Metric::getM2()
  {
    return m2;
  }

  int Metric::getNumSamples()
  {
    return n;
  }

  bool Metric::isUpdated()
  {
    return updated;
  }

  bool Metric::setUpdated(bool value)
  {
    updated = value;
  }
}
