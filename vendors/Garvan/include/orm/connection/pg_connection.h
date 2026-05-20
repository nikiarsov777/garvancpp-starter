#ifndef PG_CONNECTION_H
#define PG_CONNECTION_H

#pragma once
#include <pqxx/pqxx>
#include "db_connection.h"

class PgConnection : public DbConnection
{
public:
    PgConnection();
    PgConnection(string dbName, string dbHost, string dbPort, string dbUser, string dbPassword, string schema);
    ~PgConnection();
    // std::shared_ptr<PgConnection::Conn> execute(string query);

    std::shared_ptr<PgConnection> create(string dbName, string dbHost, string dbPort, string dbUser, string dbPassword, string schema);

    json execute(string query) override;
    json execute(const PreparedStatement& stmt) override;
    void disconnect() override;

    pqxx::connection *con;

private:
    string dbName;
    string dbHost;
    string dbPort;
    string dbUser;
    string dbPassword;

    string _execute(string query);
    string _executePrepared(const PreparedStatement& stmt);
};

#endif
