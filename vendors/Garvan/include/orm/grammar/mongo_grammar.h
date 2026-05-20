#ifndef MONGO_GRAMMAR_H
#define MONGO_GRAMMAR_H

#pragma once

#include "grammar.h"


class MongoGrammar : public Grammar {
public:

    MongoGrammar();
    ~MongoGrammar();

    std::string wrap(const std::string& value) const override {
        assertSafeMongoField(value);
        return value;
    }

    // Mongo doesn't use SQL placeholders. compileXxx returns a
    // PreparedStatement whose `sql` field holds the JSON query and
    // whose `params` is always empty.
    PreparedStatement compileSelect(const Garvan::Builder& builder) const override;
    PreparedStatement compileInsert(const Garvan::Builder& builder) const override;
    PreparedStatement compileUpdate(const Garvan::Builder &builder) const override;
    PreparedStatement compileDelete(const Garvan::Builder &builder) const override;

protected:
    // Field-name validator for BSON keys. Inherits the control-char
    // check from Grammar and additionally rejects leading '$', which
    // would otherwise be interpreted as a Mongo operator (e.g. $where,
    // $function) and let an attacker pivot a column-name into a
    // server-side JS execution / operator-injection primitive.
    static void assertSafeMongoField(const std::string& id) {
        Grammar::assertSafeIdentifier(id);
        if (!id.empty() && id[0] == '$') {
            throw std::runtime_error(
                "MongoGrammar: field name may not begin with '$'");
        }
    }
};

#endif
