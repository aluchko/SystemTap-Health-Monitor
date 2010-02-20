// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.

#include "Monitor.hxx"

#include <iostream>
#include <string>
#include <cstdlib> 
#include <stdio.h>

int main()
{
   FILE *fpipe;
   const char *command="stap ../monitors/schedtimes.stp";
   Monitor *mon = new Monitor(command);
   mon->run();
   exit(0);
}
