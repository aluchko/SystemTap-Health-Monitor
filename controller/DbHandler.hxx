// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.
#ifndef DB_HANDLER_H
#define DB_HANDLER_H 1

#include "Message.hxx"
#include <sqlite3.h>

// This class handles the interactions with the database. The database stores two primary things, the long term statistics for each metric, and a short term running snapshot of the system, consisting of a list of the various metrics collected.
namespace systemtap
{
class DbHandler
{
private:
  sqlite3* db;
  char *zErrMsg;
  int rc;
  
public:
  DbHandler() {}

  int connect();
  // store the given message in the database
  int storeMessage(Message *message);

  int exec(char* sql);
};
}
#endif
