#ifndef SQLITE_CONNECTION_H
#define SQLITE_CONNECTION_H

#pragma once

#include "db_connection.h"
#include "tools/Helper.h"
#include <sqlite3.h>
#include <memory>

class SqliteConnection : public DbConnection
{
public:
    SqliteConnection();
    SqliteConnection(string dbName, string dbHost, string dbPort, string dbUser, string dbPassword, string schema);
    ~SqliteConnection();

    std::shared_ptr<SqliteConnection> create(string dbName, string dbHost, string dbPort, string dbUser, string dbPassword, string schema);

    json execute(string query) override;
    json execute(const PreparedStatement& stmt) override;
    void disconnect() override;

    sqlite3 *con = nullptr;

private:
    string dbName;

    // Internal helpers. _execute runs raw SQL (no params). _executePrepared
    // runs a SQL string with placeholders and binds the params natively.
    string _execute(string query);
    string _executePrepared(const PreparedStatement& stmt);
};

#endif // SQLITE_CONNECTION_H
