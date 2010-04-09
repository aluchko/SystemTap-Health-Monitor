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
#include <mysql++.h>
#include <iostream>
#include <string>
#include <cstdlib> 
#include <cstdio>
#include <cstring>

// The monitor class wraps around a single SystemTap monitoring script.
namespace systemtap
{
  class Monitor
  {

  private:
    void setName(const char* name);
    const char* command;
  public:
    void run();
    Monitor(const char* monitorFile);
    static void* start_thread(void* obj);
    // start the main loop of the monitor
  };
}
#endif
