// systemtap health monitor
// Copyright (C) 2010 Aaron Luchko
//
// This file is part of systemtap, and is free software.  You can
// redistribute it and/or modify it under the terms of the GNU General
// Public License (GPL); either version 2, or (at your option) any
// later version.

#include "MetricHandler.hxx"
#include <cstring>
#include <tr1/unordered_map>

namespace systemtap
{
using namespace std;

  // this map stores MetricTypes indexed by their names
  typedef std::tr1::unordered_map<std::string, MetricType*> MetricTypeMap;
  // this map stores Metrics indexed by their MetricIds, note that a MetricId
  // is only unique within the scope of a MetricType
  typedef std::tr1::unordered_map<std::string, Metric*> MetricMap;
  // this map stores the MetricMaps corresponding to each MetricType
  typedef std::tr1::unordered_map<std::string, MetricMap*> MetricMapMap; // maybe just use map

  MetricTypeMap typeMap;
  MetricMapMap mapMap;

  MetricHandler::MetricHandler(sqlite3* database)
  {
    db = database;
    const char* mt_insert_sql = "INSERT INTO metric_type (name, min, max, def) VALUES (?1, ?2, ?3, ?4)";
    sqlite3_prepare_v2(db, mt_insert_sql, strlen(mt_insert_sql), &insert_metrictype_stmt, NULL);

    const char* find_mt_sql = "select id, min, max, def from metric_type where name = ?";
    sqlite3_prepare_v2(db, find_mt_sql, strlen(find_mt_sql), &find_metrictype_stmt, NULL);

    const char* met_insert_sql = "INSERT INTO metric (name, metric_type_id) VALUES (?1, ?2)";
    sqlite3_prepare_v2(db, met_insert_sql, strlen(met_insert_sql), &insert_metric_stmt, NULL);

    const char* find_metric_sql = "SELECT id, mean, num_samples, m2 from metric where name = ? AND metric_type_id = ?";
    sqlite3_prepare_v2(db, find_metric_sql, strlen(find_metric_sql), &find_metric_stmt, NULL);

    const char* update_metric_sql = "UPDATE metric SET mean = ?1, num_samples = ?2, m2 = ?3 WHERE id = ?4";
    sqlite3_prepare_v2(db, update_metric_sql, strlen(update_metric_sql), &update_metric_stmt, NULL);

    const char* metricvalue_insert_sql = "INSERT INTO metric_value (metric_id, time, value) VALUES (?1, ?2, ?3)";
    sqlite3_prepare_v2(db, metricvalue_insert_sql, strlen(metricvalue_insert_sql), &insert_metricvalue_stmt, NULL);

    //TODO: need to free the sql?
  }

  void MetricHandler::addMetricType(MetricType* metricType) 
  {
    std::cout << "Add metricType :" << metricType->getName() << ":" << std::endl;
    mapMap[metricType->getName()] = new MetricMap;
    typeMap[metricType->getName()] = metricType;

    // see if this metricType is already in the database 
    sqlite3_bind_text(find_metrictype_stmt, 1, metricType->getName(), -1, SQLITE_STATIC);
    int rc = sqlite3_step(find_metrictype_stmt);
    if (rc == SQLITE_DONE) 
      { // wasn't stored in db already so create it and
	// add the new metric type into the database
	sqlite3_bind_text(insert_metrictype_stmt, 1, metricType->getName(), -1, SQLITE_STATIC);
	if (metricType->isMinSet())
	  sqlite3_bind_double(insert_metrictype_stmt, 2, metricType->getMin());
	if (metricType->isMaxSet())
	  sqlite3_bind_int(insert_metrictype_stmt, 3, metricType->getMax());
	if (metricType->isDefaultSet())
	  sqlite3_bind_int(insert_metrictype_stmt, 4, metricType->getDefault());
	sqlite3_step(insert_metrictype_stmt);
	sqlite3_reset(insert_metrictype_stmt);
	
	// find the id of the new metrictype and assign it to the object
	sqlite3_reset(find_metrictype_stmt);
	sqlite3_bind_text(find_metrictype_stmt, 1, metricType->getName(), -1, SQLITE_STATIC);
	sqlite3_step(find_metrictype_stmt);
      }
    else if (rc != SQLITE_ROW)
      { 
	cerr << "Unknown Error adding metric type: " << rc << endl;
	sqlite3_reset(find_metrictype_stmt);
	return;
      }
    // if the return was an SQLITE_ROW we don't have to do anything except
    // read the id as the other qualities of the metrictype shouldn't change
    metricType->setId(sqlite3_column_int(find_metrictype_stmt,0));

    sqlite3_reset(find_metrictype_stmt);
  }

  void MetricHandler::updateMetric(char* metricTypeName, char* metricId, double time, double value)
  {
    MetricMap* metrics = mapMap[metricTypeName];
    int rc;
    Metric* metric = (*metrics)[metricId];
    if (metric == 0)
      { // metric wasn't loaded already, load now
	MetricType* metricType = typeMap[metricTypeName];
	metric = new Metric(metricType, metricId);
	(*metrics)[metricId] = metric;

	// see if this is a metric we have in the db
	sqlite3_bind_text(find_metric_stmt, 1, metric->getName(), -1, SQLITE_STATIC);
	sqlite3_bind_int(find_metric_stmt, 2, metricType->getId());
	rc = sqlite3_step(find_metric_stmt);
	if (rc == SQLITE_ROW) 
	  { // found the record in the DB, grab the historical id, mean, and std
	    metric->setMean(sqlite3_column_double(find_metric_stmt,1));
	    metric->setNumSamples(sqlite3_column_int(find_metric_stmt,2));
	    metric->setM2(sqlite3_column_double(find_metric_stmt,3));
	  }
	else if (rc == SQLITE_DONE)
	  {  // metric wasn't in the DB, new record
	    std::cout<< "Create new metric " << metricId << " " << metricType->getId() << std::endl;
	    sqlite3_bind_text(insert_metric_stmt, 1, metric->getName(), -1, SQLITE_STATIC);
	    sqlite3_bind_int(insert_metric_stmt, 2, metricType->getId());
	    do {
	      cout << "WAIT1 " << metric->getId() << " " <<metricType->getId() << endl;
	      rc = sqlite3_step(insert_metric_stmt);}
	    while (rc == SQLITE_BUSY);
	      
	    if (rc != SQLITE_DONE)
	      cerr << "ERROR inserting metric "<< metric->getId()<< " " << metric->getType()->getName() << " " << endl;
	    sqlite3_reset(insert_metric_stmt);
	    // now find the id of the new record
	    sqlite3_reset(find_metric_stmt);
	    sqlite3_step(find_metric_stmt);
	  }
	else
	  {
	    cerr << "UNKNOWN ERROR retrieving metric " << metric->getName() << endl;
	    sqlite3_reset(find_metric_stmt);
	    return;
	  }

	// set the id, this is valid whichever branch we took
	metric->setId(sqlite3_column_int(find_metric_stmt,0));
	sqlite3_reset(find_metric_stmt);
      }
    metric->update(time, value);
    //    cout << "Insert metricvalue " << metric->getName() << " for " << metric->getType()->getName() << endl;
    sqlite3_bind_int(insert_metricvalue_stmt, 1, metric->getId());
    sqlite3_bind_double(insert_metricvalue_stmt, 2, time);
    sqlite3_bind_double(insert_metricvalue_stmt, 3, value);
    do {
      cout << "WAIT2 " << metric->getId() << " " <<metric->getType()->getId() << endl;

      rc = sqlite3_step(insert_metricvalue_stmt);}
    while (rc == SQLITE_BUSY);

    if (rc != SQLITE_DONE)
      cerr << "ERROR " << rc << " inserting metric_value "<< metric->getId()<< " " << time << " " << value << endl;
    sqlite3_reset(insert_metricvalue_stmt);

  }

  void MetricHandler::persistUpdates()
  { 
    int rc;
    MetricTypeMap::iterator mtmi; 
    MetricMapMap::iterator mmmi; 
    MetricMap::iterator mmi; 
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    for (mtmi = typeMap.begin(); mtmi != typeMap.end(); mtmi++) {
      std::cout << "MetricType " << mtmi->second->getName() << std::endl;
      MetricType* metricType = mtmi->second;
      MetricMap* mm = mapMap[mtmi->first];

      for (mmi = mm->begin(); mmi != mm->end(); mmi++) {
	Metric* metric = mmi->second;
	if (metric->isUpdated())
	  {
	    cout << "Metric " << metric->getId() << " " << metric->getName() << " " << metric->getMean() << " " << metric->getNumSamples() << " " << metric->getM2() << " " << metric->getStd() << std::endl;
	    sqlite3_bind_double(update_metric_stmt, 1, metric->getMean());
	    sqlite3_bind_int(update_metric_stmt, 2, metric->getNumSamples());
	    sqlite3_bind_double(update_metric_stmt, 3, metric->getM2());
	    sqlite3_bind_int(update_metric_stmt, 4, metric->getId());
	    rc = sqlite3_step(update_metric_stmt);
	    sqlite3_reset(update_metric_stmt);
	    metric->setUpdated(false);
	  }
      }
    }
    sqlite3_exec(db, "COMMIT TRANSACTION;", 0, 0, 0);
  }
}
