#ifndef MYSQL_CONNECTION_H
#define MYSQL_CONNECTION_H

#pragma once
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <mysql_driver.h>
#include "db_connection.h"

class MysqlConnection : public DbConnection
{
public:
    MysqlConnection();
    MysqlConnection(string dbName, string dbHost, string dbPort, string dbUser, string dbPassword, string schema);
    ~MysqlConnection();

    // std::shared_ptr<MysqlConnection::Conn> execute(string query);
    std::shared_ptr<MysqlConnection> create(string dbName, string dbHost, string dbPort, string dbUser, string dbPassword, string schema);

    json execute(string query) override;
    json execute(const PreparedStatement& stmt) override;
    void disconnect() override;
    // json executeMongo(string collection, string filterJson, int limit) override {
    //     return json::array();
    // }


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
