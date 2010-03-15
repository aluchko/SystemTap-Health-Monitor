// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.

#include "DbHandler.hxx"

// A Message wraps around a single message sent from a monitor script. The Message object is designed to be a relatively simple object, simply parsing and storing the message recieved by the script.
namespace systemtap
{
  int DbHandler::connect()
  {
    rc = sqlite3_open("/tmp/test.db", &db);
    if( rc ){
      std::cout<<"Can't open database: " << sqlite3_errmsg(db) << std::endl;
    }
    return rc;
  }

  int exec(char* sql)
  {
    sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
  }

  int close()
  {
    sqlite3_close(db);
  }
}
