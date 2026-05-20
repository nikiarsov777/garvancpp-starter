#ifndef DB_FACTORY_H
#define DB_FACTORY_H

#pragma once

#include "grammar/grammar.h"
#include "grammar/postgres_grammar.h"
#include "grammar/mysql_grammar.h"
#include "grammar/mongo_grammar.h"
#include "grammar/monetdb_grammar.h"
#include "grammar/sqlite_grammar.h"

#include "connection/mysql_connection.h"
#include "connection/pg_connection.h"
#include "connection/monetdb_connection.h"
#include "connection/mongodb_connection.h"
#include "connection/sqlite_connection.h"

class DbFactory
{
public:
    DbFactory(string dbType);
    ~DbFactory();
    Grammar *getGrammar();
    void make(string dbName, string dbHost, string dbPort, string dbUser, string dbPassword, string schema);
    json execute(string query);
    json execute(const PreparedStatement& stmt);
    void discon();

private:
    DbConnection *conn;
    Grammar* grammar;

    string dbType;
};

#endif
