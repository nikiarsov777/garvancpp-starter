#ifndef MONGODB_CONNECTION_H
#define MONGODB_CONNECTION_H

#include "db_connection.h"

// ✅ Mongo C++ driver (ползваме само него)
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/collection.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/json.hpp>

#include <vector>
#include <string>
#include <memory>

using namespace std;

typedef std::vector<bsoncxx::document::value> mongo_result;

class MongoDbConnection : public DbConnection
{
public:
    MongoDbConnection();
    MongoDbConnection(string dbName, string dbHost, string dbPort,
                      string dbUser, string dbPassword, string schema);
    ~MongoDbConnection();

    json execute(string query) override;
    json execute(const PreparedStatement& stmt) override;
    void disconnect() override;

private:
    // 🔥 Mongo instance (задължителен!)
    static mongocxx::instance instance;

    unique_ptr<mongocxx::client> client;
    mongocxx::database db;

    string dbName;
    string dbHost;
    string dbPort;
    string dbUser;
    string dbPassword;

    mongo_result _execute(string query);
};

#endif // MONGODB_CONNECTION_H
