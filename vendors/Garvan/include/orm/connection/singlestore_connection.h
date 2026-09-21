#ifndef SINGLESTORE_CONNECTION_H
#define SINGLESTORE_CONNECTION_H

#pragma once
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <mysql_driver.h>
#include "db_connection.h"

// ---------------------------------------------------------------
// SingleStore / MemSQL е MySQL wire-protocol compatible. Ползваме
// libmysqlcppconn за transport (идентично на MysqlConnection), но
// пазим отделен клас за симетрия с останалите backend-и и за да
// можем в бъдеще да разклоним поведението (напр. distributed
// timeout retry, USE PARTITION hints) без да чупим MySQL пътя.
// ---------------------------------------------------------------
class SinglestoreConnection : public DbConnection
{
public:
    SinglestoreConnection();
    SinglestoreConnection(string dbName, string dbHost, string dbPort, string dbUser, string dbPassword, string schema);
    ~SinglestoreConnection();

    std::shared_ptr<SinglestoreConnection> create(string dbName, string dbHost, string dbPort, string dbUser, string dbPassword, string schema);

    json execute(string query) override;
    json execute(const PreparedStatement& stmt) override;
    void disconnect() override;

    sql::Driver *driver;
    sql::Connection *con;

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
