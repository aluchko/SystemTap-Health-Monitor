// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.

#include "Monitor.hxx"
#define DBHOST "localhost"
#define USER "health"
#define PASSWORD "password"
#define DATABASE "health"

namespace systemtap
{
  using namespace std;

  // name of the underlying monitor
  char* name;

  // all the metric types
  MetricType metricTypes[10];
  int numMetricTypes = 0;
  Monitor::Monitor(const char* monitorFile)
  {
    command = monitorFile;
  }

  void Monitor::setName(const char* scriptName) {
    name = new char[strlen(scriptName)+1];
    strcpy(name, scriptName);
  }

  void* Monitor::start_thread(void* obj)
  {
    //All we do here is call the do_work() function
    reinterpret_cast<Monitor *>(obj)->run();
  }

  void Monitor::run()
  {
    // This is the main control loop for this monitor. We let the script run and supply feedback in the form of messages. 
    FILE *fpipe;
    char line[256];

   
    mysqlpp::Connection conn(false);
    if (!conn.connect(DATABASE, DBHOST, USER, PASSWORD)){
      std::cout << "ERR" << std::endl;
      exit(1);
    }
    MetricHandler* handler = new MetricHandler(&conn);
  
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
	
	double timeStamp = atof(line); // the timestamp
	int messageOver = 0;
	mysqlpp::Transaction trans(conn);

	while (!messageOver && fgets( line, sizeof line, fpipe))
	  {	    
	    if (line[0] == '\n')
	      { // End of message block...
		messageOver = 1;
	      }
	    else {
	      char* metricTypeName = strtok(line, ":");
	      //	      cout << metricTypeName << endl;
	      char* metricName = strtok(NULL, ":");
	      //cout << metricName << endl;
	      double value = atof(strtok(NULL, ":"));
	      //cout << value << endl;
	      handler->updateMetric(metricTypeName, metricName, timeStamp, value);
	    }
	  }
	trans.commit();

	iter++;
	handler->persistUpdates();
      }
  }
} 
