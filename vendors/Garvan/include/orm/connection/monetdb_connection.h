#ifndef MONETDB_CONNECTION_H
#define MONETDB_CONNECTION_H

#include "db_connection.h"
#include "tools/Helper.h"
#include <monetdb/mapi.h>

class MonetDbConnection : public DbConnection
{
public:
    MonetDbConnection();
    MonetDbConnection(string dbName, string dbHost, string dbPort, string dbUser, string dbPassword, string schema);
    ~MonetDbConnection();
    // std::shared_ptr<PgConnection::Conn> execute(string query);

    std::shared_ptr<MonetDbConnection> create(string dbName, string dbHost, string dbPort, string dbUser, string dbPassword, string schema);

    json execute(string query) override;
    json execute(const PreparedStatement& stmt) override;
    void disconnect() override;

    Mapi con;

private:
    string dbName;
    string dbHost;
    string dbPort;
    string dbUser;
    string dbPassword;

    string _execute(string query);
};

#endif // MONETDB_CONNECTION_H
