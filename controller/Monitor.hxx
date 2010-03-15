// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.
#ifndef MONITOR_H
#define MONITOR_H 1
#include "Message.hxx"
#include "Metric.hxx"
#include "MetricType.hxx"
#include "MetricHandler.hxx"
#include <sqlite3.h>

// The monitor class wraps around a single SystemTap monitoring script.
namespace systemtap
{
class Monitor
{
private:
  sqlite3* db;
  Monitor() {}
  Message* parse();
  void setName(char* name);
public:
  Monitor(const char *script);
  // start the main loop of the monitor
  void run();
  void getName();
};
}
#endif
