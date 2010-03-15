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
#include <cstdio>
#include <cstring>   /* for all the new-fangled string functions */


namespace systemtap
{
  using namespace std;

  // name of the underlying monitor
  char* name;

  // all the metric types
  MetricType metricTypes[10];
  int numMetricTypes = 0;

  Monitor::Monitor(const char* script)
  {
    cout << script << endl;
    char *zErrMsg;
    int rc;
    rc = sqlite3_open("/tmp/test.db", &db);
    if (sqlite3_exec(db,"CREATE TABLE metric_type (id INTEGER PRIMARY KEY, name TEXT, min DOUBLE, max DOUBLE, def DOUBLE)", NULL, 0, &zErrMsg))
      cout << "ERR" << endl;
    if(sqlite3_exec(db,"CREATE TABLE metric (id INTEGER PRIMARY KEY, name TEXT, metric_type_id INTEGER, mean DOUBLE, std DOUBLE)", NULL, 0, &zErrMsg))
      cout << "ERR" << endl;
    if (sqlite3_exec(db,"CREATE TABLE metric_value (id INTEGER PRIMARY KEY, metric_id INTEGER, time double, value DOUBLE)", NULL, 0, &zErrMsg))
      cout << "ERR" << endl;
    
  }
  
  void Monitor::setName(char* scriptName) {
    name = new char[strlen(scriptName)+1];
    strcpy(name, scriptName);
  }

  void Monitor::run()
  {
    // This is the main control loop for this monitor. We let the script run and supply feedback in the form of messages. 
    FILE *fpipe;
    const char *command="stap ../monitors/schedtimes.stp";
    char line[256];
    MetricHandler* handler = new MetricHandler(db);
  
    if ( !(fpipe = (FILE*)popen(command,"r")) )
      {  // If fpipe is NULL
	perror("Problems with pipe");
	exit(1);
      }
    //    Message *message = new Message();
    enum {STARTUP, INFLOW};
    enum {TIMESTAMP, DATA};
    int msgType = STARTUP;
    int runState = TIMESTAMP;
    // look for the startup message
    if (!fgets( line, sizeof line, fpipe))
      {
      printf("ERROR\n");
      exit(1);
      }
    printf("Started at %s\n",line);
    if (!fgets( line, sizeof line, fpipe))
      {
      printf("ERROR\n");
      exit(1);
      }
    setName(line);
    printf("Script name %s\n",line);

    while ( fgets( line, sizeof line, fpipe))
      {	
	if (line[0] == '\n')
	  {
	    break;
	  }
	MetricType* type = new MetricType();
	char* metricTypeName = strtok(line, ":");
	type->setName(metricTypeName);
	char* minStr = strtok(NULL, ":");
	type->setMin(atof(minStr));
	char* maxStr = strtok(NULL, ":");
	type->setMax(atof(maxStr));
	char* defStr = strtok(NULL, ":");
	type->setDefault(atof(defStr));
	cout << type->getName() << " min " << type->getMin()<< endl;
	handler->addMetricType(type);
      }
    int iter = 0;
    while ( fgets( line, sizeof line, fpipe))
      { // This loop reads the first line of a message (the timestamp) than
	// passes control to the inner loop that reads the list of metrics
	cout << "HERE "<<line<<endl;
	// the timestamp
	double timeStamp = atof(line);
	while ( fgets( line, sizeof line, fpipe))
	  {	    
	    if (line[0] == '\n')
	      { // End of message block... what to do
		printf("END OF MESSAGE\n");
		break;  // break from the inner message loop to the outer loop
	      }
	    char* metricTypeName = strtok(line, ":");
	    cout << metricTypeName << endl;
	    char* metricName = strtok(NULL, ":");
	    cout << metricName << endl;
	    double value = atof(strtok(NULL, ":"));
	    cout << value << endl;
	    handler->updateMetric(metricTypeName, metricName, timeStamp, value);
	  }
	//	pclose(fpipe);
	iter++;
	if (iter > 1)
	  break;
      }
    handler->persistUpdates();
  }
} 
