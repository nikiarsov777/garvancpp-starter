#ifndef POSTGRESQL_GRAMMAR_H
#define POSTGRESQL_GRAMMAR_H

#pragma once

#include "grammar.h"

class PostgreSqlGrammar : public Grammar {
public:

    PostgreSqlGrammar();
    ~PostgreSqlGrammar();

    string wrap(const string& value) const override;

    // PostgreSQL uses $1, $2, ... placeholders.
    string placeholder(size_t index) const override;

    PreparedStatement compileSelect(const Garvan::Builder& builder) const override;

    PreparedStatement compileInsert(const Garvan::Builder& builder) const override;
    PreparedStatement compileUpdate(const Garvan::Builder &builder) const override;
    PreparedStatement compileDelete(const Garvan::Builder &builder) const override;
};
#endif
