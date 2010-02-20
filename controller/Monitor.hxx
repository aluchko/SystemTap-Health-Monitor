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

class Monitor
{
private:
  Monitor() {}
  Message parse();
public:
  Monitor(const char *script);
  void run();
};
#endif
