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

using namespace std;
Monitor::Monitor(const char* script)
{
  cout << script << endl;
}

void Monitor::run()
{
  FILE *fpipe;
  const char *command="stap ../monitors/schedtimes.stp";
  char line[256];
  
  if ( !(fpipe = (FILE*)popen(command,"r")) )

    {  // If fpipe is NULL
      perror("Problems with pipe");
      exit(1);
    }

  while ( fgets( line, sizeof line, fpipe))
    {
      if (line[0] == 'E')
	{ // End of line
	}
      printf("%s", line);
    }
  pclose(fpipe);
}
