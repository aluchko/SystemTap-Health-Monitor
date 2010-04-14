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
      query.execute("CREATE TABLE metric_type (id INTEGER PRIMARY KEY AUTO_INCREMENT, name TEXT NOT NULL, min DOUBLE, max DOUBLE, def DOUBLE)");
    }  
    catch (const mysqlpp::BadQuery& er)
      { 
	if (er.errnum() == ER_TABLE_EXISTS_ERROR) {}//this is fine
	else
	  cerr << "Query1 error: " << er.errnum() << ":" << er.what() << endl;
      }
    
    try {
      query.execute("CREATE TABLE metric (id INTEGER PRIMARY KEY AUTO_INCREMENT, name TEXT NOT NULL, metric_type_id INTEGER NOT NULL, mean DOUBLE, num_samples INTEGER, m2 DOUBLE, current_value DOUBLE NOT NULL, time DOUBLE NOT NULL)");
    }  
    catch (const mysqlpp::BadQuery& er)
      { 
	if (er.errnum() == ER_TABLE_EXISTS_ERROR) {}//this is fine
	else
	  cerr << "Query2 error: " << er.errnum() << ":" << er.what() << endl;
      }
    
    try {
      query.execute("CREATE TABLE metric_value (id INTEGER PRIMARY KEY AUTO_INCREMENT, metric_id INTEGER NOT NULL, time double NOT NULL, value DOUBLE NOT NULL, FOREIGN KEY (metric_id) REFERENCES metric(id))");
    }  
    catch (const mysqlpp::BadQuery& er)
      { 
	if (er.errnum() == ER_TABLE_EXISTS_ERROR) {}//this is fine
	else
	  cerr << "Query3 error: " << er.errnum() << ":" << er.what() << endl;
      }
    
    try {
      query.execute("CREATE INDEX mv_order ON metric_value (metric_id, time desc)");
    }  
    catch (const mysqlpp::BadQuery& er)
      { 
	if (er.errnum() ==ER_DUP_KEYNAME) {}//this is fine
	else
	  cerr << "Query4 error: " << er.errnum() << ":" << er.what() << endl;
       }
    try {
      query.execute("CREATE INDEX m_metric_type_id ON metric (metric_type_id)");
    }  
    catch (const mysqlpp::BadQuery& er)
      { 
	if (er.errnum() ==ER_DUP_KEYNAME) {}//this is fine
	else
	  cerr << "Query4 error: " << er.errnum() << ":" << er.what() << endl;
       }
    conn.disconnect();
    
    vector<const char*> monitors(2);
    vector<pthread_t> threads(2);
    monitors[0] = "stap ../monitors/schedtimes.stp";
    monitors[1] = "stap ../monitors/iotop.stp";
    for (int i = 0; i < 2; i++)
      {
	Monitor* monitor = new Monitor(monitors[i]);
	pthread_t thread;
	pthread_create( &thread, NULL, &Monitor::start_thread, monitor);
	threads[i] = thread;
      }
    for (int i = 0; i < 2; i++)
      pthread_join(threads[i], NULL);
    
    exit(0);
  }
  else {
    std::cerr << "DB connection failed: " << conn.error() << std::endl;
    return 1; 
  }
}
