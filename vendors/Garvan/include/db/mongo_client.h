#pragma once
#include "DbClient.h"
#include <mongoc/mongoc.h> // Директно C драйвера

class MongoClient //: public DbClient
{
private:
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    string dbName;
public:
    MongoClient(string uri, string db);
    ~MongoClient();
    
    // json execute(string query) { return json::array(); }
    string executeMongo(string collection, string filterJson, int limit);
};

