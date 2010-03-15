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

    const char* find_mt_sql = "select id from metric_type where name = ?";
    sqlite3_prepare_v2(db, find_mt_sql, strlen(find_mt_sql), &find_metrictype_stmt, NULL);

    const char* met_insert_sql = "INSERT INTO metric (name, metric_type_id) VALUES (?1, ?2)";
    sqlite3_prepare_v2(db, met_insert_sql, strlen(met_insert_sql), &insert_metric_stmt, NULL);

    const char* find_metric_sql = "select id from metric where name = ?";
    sqlite3_prepare_v2(db, find_metric_sql, strlen(find_metric_sql), &find_metric_stmt, NULL);

    const char* update_metric_sql = "UPDATE metric SET mean = ?1, std = ?2 WHERE id = ?3";
    sqlite3_prepare_v2(db, update_metric_sql, strlen(update_metric_sql), &update_metric_stmt, NULL);

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
    if (rc == SQLITE_ROW)
      {
	//read the row
      }
    else if (rc == SQLITE_DONE) 
      { // new MetricType, create it
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
	metricType->setId(sqlite3_column_int(find_metrictype_stmt,0));
      }
    else 
      {
	cerr << "Unknown Error adding metric type: " << rc << endl;
      }
    sqlite3_reset(find_metrictype_stmt);
  }

  void MetricHandler::updateMetric(char* metricTypeName, char* metricId, int time, double value)
  {
    MetricMap* metrics = mapMap[metricTypeName];
    
    Metric* metric = (*metrics)[metricId];
    if (metric == 0)
      {
	MetricType* metricType = typeMap[metricTypeName];
	metric = new Metric(metricType, metricId);
	(*metrics)[metricId] = metric;
	std::cout<< "Create new metric " << metricId << " " << metricType->getId() << std::endl;

	// insert the new metric into the DB
	sqlite3_bind_text(insert_metric_stmt, 1, metric->getName(), -1, SQLITE_STATIC);
	sqlite3_bind_int(insert_metric_stmt, 2, metricType->getId());
	sqlite3_step(insert_metric_stmt);
	sqlite3_reset(insert_metric_stmt);
	
	sqlite3_bind_text(find_metric_stmt, 1, metric->getName(), -1, SQLITE_STATIC);
	sqlite3_step(find_metric_stmt);
	metric->setId(sqlite3_column_int(find_metric_stmt,0));
	sqlite3_reset(find_metric_stmt);

      }
    metric->update(time, value);
  }

  void MetricHandler::persistUpdates()
  { 
    int rc;
    MetricTypeMap::iterator mtmi; 
    MetricMapMap::iterator mmmi; 
    MetricMap::iterator mmi; 
    for (mtmi = typeMap.begin(); mtmi != typeMap.end(); mtmi++) {
      std::cout << "MetricType " << mtmi->second->getName() << std::endl;
      MetricType* metricType = mtmi->second;
      MetricMap* mm = mapMap[mtmi->first];

      for (mmi = mm->begin(); mmi != mm->end(); mmi++) {
	Metric* metric = mmi->second;
	cout << "Metric " << metric->getId() << " " << metric->getName() << " " << metric->getMean() << " " << mmi->second->getStd() << std::endl;
	sqlite3_bind_double(update_metric_stmt, 1, metric->getMean());
	sqlite3_bind_double(update_metric_stmt, 2, metric->getStd());
	sqlite3_bind_int(update_metric_stmt, 3, metric->getId());
	rc = sqlite3_step(update_metric_stmt);
	cout << rc << endl;
	sqlite3_reset(update_metric_stmt);	
      }
    }
  }
}
