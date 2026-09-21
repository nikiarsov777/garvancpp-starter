#ifndef SINGLESTORE_GRAMMAR_H
#define SINGLESTORE_GRAMMAR_H

#pragma once

#include "mysql_grammar.h"

// ---------------------------------------------------------------
// SingleStore/MemSQL grammar. SingleStore е MySQL wire-protocol
// compatible: backticks за identifiers, `?` positional placeholders,
// JSON_ARRAYAGG / JSON_OBJECT (7.5+). В първата ревизия няма
// override-и — всичко се inherit-ва от MySqlGrammar. Класът съществува
// като anchor point за бъдещи divergence-и (SHARD KEY hints,
// distributed-only DDL, TIMEOUT_MS suffix и т.н.).
// ---------------------------------------------------------------
class SinglestoreGrammar : public MySqlGrammar {
public:
    SinglestoreGrammar();
    ~SinglestoreGrammar();
};

#endif
