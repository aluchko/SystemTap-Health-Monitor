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
  using namespace mysqlpp;

  // this map stores MetricTypes indexed by their names
  typedef std::tr1::unordered_map<std::string, MetricType*> MetricTypeMap;
  // this map stores Metrics indexed by their MetricIds, note that a MetricId
  // is only unique within the scope of a MetricType
  typedef std::tr1::unordered_map<std::string, Metric*> MetricMap;
  // this map stores the MetricMaps corresponding to each MetricType
  typedef std::tr1::unordered_map<std::string, MetricMap*> MetricMapMap; // maybe just use map

  MetricTypeMap typeMap;
  MetricMapMap mapMap;

  MetricHandler::MetricHandler(mysqlpp::Connection *connection)
  {
    conn = connection;
    insert_metrictype_stmt = new mysqlpp::Query::Query(conn, true, "INSERT INTO metric_type (name, min, max, def) VALUES (%0q:name, %1:min, %2:max, %3:def)");

    insert_metrictype_stmt->parse();

    //find_metrictype_stmt = new mysqlpp::Query::Query(conn, true, "select id, min, max, def from metric_type where name = 'usage'");
    find_metrictype_stmt = new mysqlpp::Query::Query(conn, true, "select id, min, max, def from metric_type where name = %0q:name");
    find_metrictype_stmt->parse();

    insert_metric_stmt = new mysqlpp::Query::Query(conn, true, "INSERT INTO metric (name, metric_type_id, current_value, time) VALUES (%0q:name, %1:metric_type_id, %2:current_value, %3:time)");
    insert_metric_stmt->parse();

    find_metric_stmt = new mysqlpp::Query::Query(conn, true, "SELECT id, mean, num_samples, m2 from metric where metric_type_id = %1:metric_type_id AND name = %0q");
    //    find_metric_stmt = new mysqlpp::Query::Query(conn, true, "SELECT id, mean, num_samples, m2 from metric where name = %0q");
    find_metric_stmt->parse();

    update_metric_stmt = new mysqlpp::Query::Query(conn, true, "UPDATE metric SET mean = %0:mean, num_samples = %1:num_samples, m2 = %2:m2, current_value = %3:current_value, time = %4:time WHERE id = %5:id");
    update_metric_stmt->parse();

    insert_metricvalue_stmt = new mysqlpp::Query::Query(conn, true, "INSERT INTO metric_value (metric_id, time, value) VALUES (%0:metric_id, %1:time, %2:value)");
    insert_metricvalue_stmt->parse();

  }

  void MetricHandler::addMetricType(MetricType* metricType) 
  {
    std::cout << "Add metricType :" << metricType->getName() << ":" << std::endl;
    mapMap[metricType->getName()] = new MetricMap;
    typeMap[metricType->getName()] = metricType;

    // see if this metricType is already in the database 
    StoreQueryResult res = find_metrictype_stmt->store(metricType->getName());
    if (res.num_rows() == 0)
      {
	// wasn't stored in db already so create it and
	// add the new metric type into the database
	
	// cleaner way to do this?
	cout << "PRE"  << endl;

	insert_metrictype_stmt->execute(metricType->getName(),(metricType->isMinSet() ? str(metricType->getMin()) : "NULL"), (metricType->isMaxSet() ? str(metricType->getMax()) : "NULL"), (metricType->isDefaultSet() ? str(metricType->getDefault()) : "NULL"));
	cout << "INSERT" << endl;
	  
	// find the id of the new metrictype and assign it to the object
	res = find_metrictype_stmt->store(metricType->getName());
      }

    cout << "HERE " << endl;
    // if the return was an SQLITE_ROW we don't have to do anything except
    // read the id as the other qualities of the metrictype shouldn't change
    metricType->setId(res[0]["id"]);    
    cout << "OUT" << endl;
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
	cout<<"Q :"<<metric->getName() << ":"<<metricType->getId()<<":" << endl;
	// need because of overloaded store function
	SQLQueryParms p;
	p.set(metric->getName(), metricType->getId());
	StoreQueryResult res = find_metric_stmt->store(p);
	cout<<"W" << endl;
	if (res.num_rows()) 
	  { // found the record in the DB, grab the historical id, mean, and std
	    try {
	      metric->setMean(res[0]["mean"]);
	    } catch (const mysqlpp::BadConversion& er){ } //NULL, just don't set
	    try {
	      metric->setNumSamples(res[0]["num_samples"]);
	    } catch (const mysqlpp::BadConversion& er){ } //NULL, just don't set
	    try {
	      metric->setM2(res[0]["m2"]);
	    } catch (const mysqlpp::BadConversion& er){ } //NULL, just don't set
	  }
	else
	  {  // metric wasn't in the DB, new record
	    std::cout<< "Create new metric " << metricId << " " << metricType->getId() << std::endl;
	    SQLQueryParms p2;
	    p2.set(metric->getName(), metricType->getId(), value, time);
	    insert_metric_stmt->execute(p2);
	    res = find_metric_stmt->store(p);
	  }

	// set the id, this is valid whichever branch we took
	metric->setId(res[0]["id"]);
      }
    metric->update(time, value);
    cout << "Insert metricvalue " << metric->getName() << " for " << metric->getType()->getName() << endl;
    insert_metricvalue_stmt->execute(metric->getId(), time, value);

  }

  void MetricHandler::persistUpdates()
  { 
    int rc;
    MetricTypeMap::iterator mtmi; 
    MetricMapMap::iterator mmmi; 
    MetricMap::iterator mmi; 

    Transaction trans(*conn);

    for (mtmi = typeMap.begin(); mtmi != typeMap.end(); mtmi++) {
      std::cout << "MetricType " << mtmi->second->getName() << std::endl;
      MetricType* metricType = mtmi->second;
      MetricMap* mm = mapMap[mtmi->first];

      for (mmi = mm->begin(); mmi != mm->end(); mmi++) {
	Metric* metric = mmi->second;
	if (metric->isUpdated())
	  {
	    cout << "MT " << metric->getType()->getName()<<"Metric " << metric->getId() << " " << metric->getName() << " " << metric->getMean() << " " << metric->getNumSamples() << " " << metric->getM2() << " " << metric->getStd() << std::endl;
	    update_metric_stmt->execute(metric->getMean(), metric->getNumSamples(), metric->getM2(), metric->getCurrentValue(), metric->getTime(), metric->getId());
	    metric->setUpdated(false);
	  }
      }
    }
    trans.commit();
  }

  inline std::string MetricHandler::str(double value)
  {
   std::ostringstream oss;
   oss << value;
   return oss.str();
  } 
}
