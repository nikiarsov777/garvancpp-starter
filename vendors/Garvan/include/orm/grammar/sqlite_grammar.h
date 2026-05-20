#ifndef SQLITE_GRAMMAR_H
#define SQLITE_GRAMMAR_H

#pragma once

#include "grammar.h"


class SqliteGrammar : public Grammar {
public:
    SqliteGrammar();
    ~SqliteGrammar();

    std::string wrap(const std::string& value) const override;

    PreparedStatement compileSelect(const Garvan::Builder& builder) const override;
    PreparedStatement compileInsert(const Garvan::Builder& builder) const override;
    PreparedStatement compileUpdate(const Garvan::Builder& builder) const override;
    PreparedStatement compileDelete(const Garvan::Builder& builder) const override;
};

#endif
