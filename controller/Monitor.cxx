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

  // all the metric types
  MetricType metricTypes[10];
  int numMetricTypes = 0;
  Monitor::Monitor(const char* monitorFile)
  {
    command = new char[strlen(monitorFile)+1];
    strcpy(command, monitorFile);
  }

  Monitor::~Monitor()
  {
    delete [] name;
    delete [] command;
  }

  void Monitor::setName(const char* scriptName)
  {
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

    mysql_library_init(0, NULL, NULL);

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
	regex_t expr;
	const char* pattern = "([^:]*):([^:]*):([^:]*):([^:]*)";
	regmatch_t matches[8];
	regcomp(&expr, pattern, REG_EXTENDED | REG_ICASE);

	// TODO: syntax check here
	int res = regexec(&expr, line, 5, matches, 0);
	MetricType* type = new MetricType();

	int start;
	int end = matches[1].rm_eo;
	char* metricTypeName = new char[end+1];
	metricTypeName[end] = '\0';
	type->setName(strncpy(metricTypeName,line,end));
	delete [] metricTypeName;

	start = matches[2].rm_so;
	end = matches[2].rm_eo;
	if (start != end)
	  type->setMin(atof((line + start)));

	start = matches[3].rm_so;
	end = matches[3].rm_eo;
	if (start != end)
	  type->setMax(atof((line + start)));

	start = matches[4].rm_so;
	end = matches[4].rm_eo;
	if (start != end)
	  type->setDefault(atof((line + start)));

	handler->addMetricType(type);
	regfree(&expr);

	//	delete [] pattern;
	//	delete &expr;
	//delete [] &matches;
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
    delete[] &line;
    cout << "CLOSED" << endl;
    mysql_library_end(); // call to C api to clean up memory leaks
  }
} 
