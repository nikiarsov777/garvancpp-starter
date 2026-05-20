#ifndef MYSQL_GRAMMAR_H
#define MYSQL_GRAMMAR_H

#pragma once

#include "grammar.h"

class MySqlGrammar : public Grammar {
public:

    MySqlGrammar();
    ~MySqlGrammar();

    string wrap(const string& value) const override;

     PreparedStatement compileSelect(const Garvan::Builder& builder) const override;

     PreparedStatement compileInsert(const Garvan::Builder& builder) const override;
     PreparedStatement compileUpdate(const Garvan::Builder& builder) const override;
     PreparedStatement compileDelete(const Garvan::Builder& builder) const override;

protected:
    // Inherits "?" placeholder from base. escapeValue is no longer
    // needed for the bound-parameter path; values flow through the
    // mysqlcppconn prepared-statement API in MysqlConnection.
};

#endif
