#ifndef GARVAN_GRAMMAR_H
#define GARVAN_GRAMMAR_H

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <algorithm>
#include <array>
#include <cctype>
#include <ranges>
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
    static void assertSafeIdentifier(std::string_view id)
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
    //
    // Allowlist е `constexpr std::array<string_view>` — не се пази
    // heap-allocated hash-set в статична памет и не се плаща
    // hashing overhead per call. При ~20 елемента linear search през
    // ranges::find е бърз.
    // ---------------------------------------------------------------
    [[nodiscard]] static string sanitizeOperator(std::string_view op)
    {
        static constexpr std::array<std::string_view, 23> ALLOWED_OPS{{
            "=", "!=", "<>", "<", "<=", ">", ">=",
            "LIKE", "NOT LIKE", "ILIKE", "NOT ILIKE",
            "IS", "IS NOT",
            "IN", "NOT IN",
            "BETWEEN", "NOT BETWEEN",
            "GLOB", "NOT GLOB",
            "MATCH", "NOT MATCH",
            "REGEXP", "NOT REGEXP"
        }};

        const std::size_t a = op.find_first_not_of(" \t\r\n");
        const std::size_t b = op.find_last_not_of(" \t\r\n");
        if (a == std::string_view::npos) {
            throw runtime_error("Grammar: empty operator");
        }
        std::string trimmed{op.substr(a, b - a + 1)};

        std::string upper = trimmed;
        std::transform(upper.begin(), upper.end(), upper.begin(),
                       [](unsigned char c){ return std::toupper(c); });

        if (std::ranges::find(ALLOWED_OPS, std::string_view{trimmed}) != ALLOWED_OPS.end())
            return trimmed;
        if (std::ranges::find(ALLOWED_OPS, std::string_view{upper}) != ALLOWED_OPS.end())
            return upper;

        throw runtime_error("Grammar: rejected operator: '" + std::string(op) + "'");
    }

    // ORDER BY allowlist (identifiers, commas, dots, spaces, ASC/DESC keywords).
    // ORDER BY positions are not bindable.
    [[nodiscard]] static std::string sanitizeOrderBy(std::string_view order)
    {
        for (char c : order) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (!(std::isalnum(uc) || c == '_' || c == ',' || c == '.'
                  || c == ' ' || c == '\t')) {
                throw runtime_error("Grammar: invalid ORDER BY characters: '"
                                    + std::string(order) + "'");
            }
        }
        return std::string(order);
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

#endif // GARVAN_GRAMMAR_H
