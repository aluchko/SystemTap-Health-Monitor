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

/* MySQL Connector/C++ specific headers */
#include <mysql++.h>
#include <mysqld_error.h>

#define DBHOST "localhost"
#define USER "health"
#define PASSWORD "password"
#define DATABASE "health"

#define NUMOFFSET 100
#define COLNAME 200

using namespace systemtap;
using namespace std;

int main()
{
  mysqlpp::Connection conn;
  if (conn.connect(DATABASE, DBHOST, USER, PASSWORD)){
    
    mysqlpp::Query query = conn.query();

    try {
      query.execute("CREATE TABLE metric_type (id INTEGER PRIMARY KEY AUTO_INCREMENT, name TEXT, min DOUBLE, max DOUBLE, def DOUBLE)");
    }  
    catch (const mysqlpp::BadQuery& er)
      { 
	if (er.errnum() == ER_TABLE_EXISTS_ERROR) {}//this is fine
	else
	  cerr << "Query1 error: " << er.errnum() << ":" << er.what() << endl;
      }
    
    try {
      query.execute("CREATE TABLE metric (id INTEGER PRIMARY KEY AUTO_INCREMENT, name TEXT, metric_type_id INTEGER, mean DOUBLE, num_samples INTEGER, m2 DOUBLE)");
    }  
    catch (const mysqlpp::BadQuery& er)
      { 
	if (er.errnum() == ER_TABLE_EXISTS_ERROR) {}//this is fine
	else
	  cerr << "Query2 error: " << er.errnum() << ":" << er.what() << endl;
      }
    
    try {
      query.execute("CREATE TABLE metric_value (id INTEGER PRIMARY KEY AUTO_INCREMENT, metric_id INTEGER, time double, value DOUBLE)");
    }  
    catch (const mysqlpp::BadQuery& er)
      { 
	if (er.errnum() == ER_TABLE_EXISTS_ERROR) {}//this is fine
	else
	  cerr << "Query3 error: " << er.errnum() << ":" << er.what() << endl;
      }
    
    try {
      query.execute("CREATE INDEX mvorder ON metric_value (metric_id, time desc)");
    }  
    catch (const mysqlpp::BadQuery& er)
      { 
	if (er.errnum() ==ER_DUP_KEYNAME) {}//this is fine
	else
	  cerr << "Query4 error: " << er.errnum() << ":" << er.what() << endl;
       }
    conn.disconnect();
    
    FILE *fpipe;
    
    pthread_t thread1, thread2;
    Monitor* mon = new Monitor("stap ../monitors/schedtimes.stp");
    Monitor* mon2 = new Monitor("stap ../monitors/iotop.stp");
    
    pthread_create( &thread1, NULL, &Monitor::start_thread, mon);
    pthread_create( &thread2, NULL, &Monitor::start_thread, mon2);
    pthread_join( thread1, NULL);
    pthread_join( thread2, NULL);
    exit(0);
  }
  else {
    std::cerr << "DB connection failed: " << conn.error() << std::endl;
    return 1; 
  }
}
