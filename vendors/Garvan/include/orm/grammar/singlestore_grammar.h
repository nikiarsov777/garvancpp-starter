#ifndef SINGLESTORE_GRAMMAR_H
#define SINGLESTORE_GRAMMAR_H

#pragma once

#include "mysql_grammar.h"

// ---------------------------------------------------------------
// SingleStore/MemSQL grammar. SingleStore е MySQL wire-protocol
// compatible (backticks, `?` placeholders), но JSON-construction
// каталогът е различен: няма `JSON_OBJECT` / `JSON_ARRAYAGG`
// (MySQL 5.7/8.0 built-ins), а ползва `JSON_BUILD_OBJECT(k,v,...)`
// и `JSON_AGG(expr)`. Затова `compileSelect` е override-нат —
// всичко останало (WHERE / JOIN / LIMIT / identifier wrap) идва
// от MySqlGrammar.
// ---------------------------------------------------------------
class SinglestoreGrammar : public MySqlGrammar {
public:
    SinglestoreGrammar();
    ~SinglestoreGrammar();

    PreparedStatement compileSelect(const Garvan::Builder& builder) const override;
};

#endif
