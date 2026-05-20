#ifndef GRAMMAR_H
#define GRAMMAR_H

#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <unordered_set>
#include "tools/JsonValue.h"
#include <map>

using json = JsonValue;
using namespace std;

// forward declaration
namespace Garvan {
    struct WhereClause {
        string column;
        string op;
        string value;
    };
    class Builder;
}

// ---------------------------------------------------------------
// PreparedStatement carries a SQL string with backend-specific
// placeholders (e.g. "?" for SQLite/MySQL/MonetDB, "$1" for Postgres)
// plus the values that should be bound to those placeholders by the
// connection. This decouples query construction (Grammar) from
// parameter binding (Connection) and removes inline-value SQL strings
// from the safety surface entirely.
// ---------------------------------------------------------------
struct PreparedStatement {
    string sql;
    vector<json> params;
};

class Grammar {
public:
    virtual ~Grammar() = default;

    // escape identifiers (table, column)
    virtual string wrap(const string& value) const = 0;

    // Backend-specific placeholder for the i-th bound parameter (0-based).
    // Default is "?" which is correct for SQLite, MySQL, MonetDB. Postgres
    // overrides to produce "$1", "$2", ...
    virtual string placeholder(size_t index) const
    {
        (void)index;
        return "?";
    }

    // Compile methods now return a PreparedStatement: an SQL string
    // with placeholders plus the params that should be bound.
    virtual PreparedStatement compileSelect(const Garvan::Builder& builder) const = 0;
    virtual PreparedStatement compileInsert(const Garvan::Builder& builder) const = 0;
    virtual PreparedStatement compileUpdate(const Garvan::Builder& builder) const = 0;
    virtual PreparedStatement compileDelete(const Garvan::Builder& builder) const = 0;


protected:
    // ---------------------------------------------------------------
    // Escape a value for inline inclusion inside a SINGLE-QUOTED SQL
    // literal. Kept for backends that cannot use native bindings (e.g.
    // MonetDB's awkward C-API binding) and need to substitute values
    // into the SQL text at the connection layer.
    // ---------------------------------------------------------------
    virtual string escapeValue(const string& s) const
    {
        string result;
        result.reserve(s.size());
        for (char c : s) {
            if (c == '\'') result += "''";
            else result += c;
        }
        return result;
    }

    // ---------------------------------------------------------------
    // Defense-in-depth identifier check. Identifier escaping (wrap)
    // already neutralises the quote character used by each backend,
    // but a NUL or other ASCII control character has no legitimate
    // place in an SQL identifier or BSON key and frequently breaks
    // C-string-based driver APIs in surprising ways. Reject early.
    // ---------------------------------------------------------------
    static void assertSafeIdentifier(const string& id)
    {
        if (id.empty()) {
            throw runtime_error("Grammar: empty identifier");
        }
        for (unsigned char c : id) {
            if (c == 0 || c < 0x20 || c == 0x7F) {
                throw runtime_error(
                    "Grammar: identifier contains a control character");
            }
        }
    }

    // ---------------------------------------------------------------
    // Operator allowlist for WHERE clauses. WhereClause::op is
    // user-supplied via Builder::where(field, op, value). We never
    // bind operators (they aren't bindable), so an allowlist is the
    // only safe option.
    // ---------------------------------------------------------------
    static string sanitizeOperator(const string& op)
    {
        size_t a = op.find_first_not_of(" \t\r\n");
        size_t b = op.find_last_not_of(" \t\r\n");
        if (a == string::npos) {
            throw runtime_error("Grammar: empty operator");
        }
        string trimmed = op.substr(a, b - a + 1);

        string upper = trimmed;
        transform(upper.begin(), upper.end(), upper.begin(),
                  [](unsigned char c){ return std::toupper(c); });

        static const unordered_set<string> allowed = {
            "=", "!=", "<>", "<", "<=", ">", ">=",
            "LIKE", "NOT LIKE", "ILIKE", "NOT ILIKE",
            "IS", "IS NOT",
            "IN", "NOT IN",
            "BETWEEN", "NOT BETWEEN",
            "GLOB", "NOT GLOB",
            "MATCH", "NOT MATCH",
            "REGEXP", "NOT REGEXP"
        };

        if (allowed.count(trimmed) > 0) return trimmed;
        if (allowed.count(upper) > 0)   return upper;

        throw runtime_error("Grammar: rejected operator: '" + op + "'");
    }

    // ORDER BY allowlist (identifiers, commas, dots, spaces, ASC/DESC keywords).
    // ORDER BY positions are not bindable.
    static string sanitizeOrderBy(const string& order)
    {
        for (char c : order) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (!(std::isalnum(uc) || c == '_' || c == ',' || c == '.'
                  || c == ' ' || c == '\t')) {
                throw runtime_error("Grammar: invalid ORDER BY characters: '" + order + "'");
            }
        }
        return order;
    }

    // Compile a WHERE clause using placeholders. Each value is appended
    // to `params`; the operator passes through the allowlist.
    virtual string compileWheres(const vector<Garvan::WhereClause>& wheres,
                                 vector<json>& params) const
    {
        if (wheres.empty()) return "";

        string sql = " WHERE ";

        for (size_t i = 0; i < wheres.size(); ++i)
        {
            const auto& w = wheres[i];
            const string op = sanitizeOperator(w.op);

            sql += wrap(w.column) + " " + op + " " + placeholder(params.size());
            params.push_back(json(w.value));

            if (i != wheres.size() - 1)
                sql += " AND ";
        }

        return sql;
    }

};

#endif
