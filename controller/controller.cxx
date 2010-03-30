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
#include <pthread.h>

using namespace systemtap;

int main()
{
  FILE *fpipe;
  sqlite3* db;

  //create the database
  char *zErrMsg;
  int rc;
  // TODO: need to look at error codes to exclude already created
  rc = sqlite3_open("/tmp/test.db", &db);
  if (sqlite3_exec(db,"CREATE TABLE metric_type (id INTEGER PRIMARY KEY, name TEXT, min DOUBLE, max DOUBLE, def DOUBLE)", NULL, 0, &zErrMsg)){}
  if(sqlite3_exec(db,"CREATE TABLE metric (id INTEGER PRIMARY KEY, name TEXT, metric_type_id INTEGER, mean DOUBLE, num_samples INTEGER, m2 DOUBLE)", NULL, 0, &zErrMsg)){}
  if (sqlite3_exec(db,"CREATE TABLE metric_value (id INTEGER PRIMARY KEY, metric_id INTEGER, time double, value DOUBLE)", NULL, 0, &zErrMsg)){}
  if (sqlite3_exec(db,"CREATE INDEX mvorder ON metric_value (metric_id, time desc)", NULL, 0, &zErrMsg)){}
  sqlite3_close(db); // can't share connection with other threads

  pthread_t thread1, thread2;
  Monitor* mon = new Monitor("stap ../monitors/schedtimes.stp");
  Monitor* mon2 = new Monitor("stap ../monitors/s2.stp");

  pthread_create( &thread1, NULL, &Monitor::start_thread, mon);
  pthread_create( &thread2, NULL, &Monitor::start_thread, mon2);
  pthread_join( thread1, NULL);
  pthread_join( thread2, NULL);
  exit(0);
}
