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
        // Logical connector applied BEFORE this clause when it's not
        // the first in the WHERE list. "AND" (default, backward-
        // compatible) or "OR". Populated by Builder::where /
        // Builder::orWhere; sanitized by Grammar::compileWheres.
        string connector = "AND";
        // For IN / NOT IN clauses — each element is bound as its own
        // placeholder and `value` is ignored. Empty for scalar
        // comparisons; populated by Builder::whereIn family.
        vector<string> values;
    };

    // ---------------------------------------------------------------
    // JoinClause — explicit, SQL-shaped JOIN metadata populated by
    // `Builder::join / leftJoin / rightJoin / innerJoin / crossJoin`.
    //
    // Independent от `Builder::joinModel` (което пази ORM-relations
    // от `hasOne / hasMany / belongsTo / ...`) — там ще ходят
    // eager-load / eloquent-style JOIN-и, а JoinClause е ниско-нивовият
    // fluent SQL-join API.
    // ---------------------------------------------------------------
    struct JoinClause {
        enum class Type { Inner, Left, Right, Cross };
        Type        type = Type::Inner;
        std::string table;         // right-hand table name (unqualified)
        std::string alias;         // optional table alias (empty = none)
        std::string leftColumn;    // qualified or unqualified: "t.c" or "c"
        std::string op = "=";      // comparison operator
        std::string rightColumn;   // qualified or unqualified
    };

    // ---------------------------------------------------------------
    // SelectClause — typed projection element. `Column` and `Aliased`
    // flow through the identifier-wrapping path (safe by construction).
    // `Raw` embeds arbitrary SQL text verbatim — callers MUST guard
    // the expression against injection (allowlist / no user input).
    // ---------------------------------------------------------------
    enum class SelectKind { Column, Aliased, Raw };
    struct SelectClause {
        SelectKind  kind = SelectKind::Column;
        std::string expr;   // identifier for Column/Aliased; SQL for Raw
        std::string alias;  // present for Aliased and Raw-with-alias
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

    // Backend-family discriminator за raw() API-то. SQL backends
    // (Postgres / MySQL / SQLite / MonetDB) връщат `true`; NoSQL
    // backends (Mongo) — `false`. Ползва се от `Builder::executeRaw()`,
    // за да реши дали `raw(sql,...)` е валидна операция или трябва да
    // премине през `rawJson(envelope)`.
    virtual bool isSql() const { return true; }

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

    // ---------------------------------------------------------------
    // Wrap a possibly-qualified identifier of the form "table.column".
    // Splits on a single '.' and quotes each side independently via
    // wrap(). Falls back to wrap(id) if there's no dot. Used from
    // compileJoins() so the emitted SQL binds columns to the correct
    // side of the join.
    // ---------------------------------------------------------------
    virtual string wrapQualified(const string& id) const
    {
        auto dot = id.find('.');
        if (dot == string::npos) return wrap(id);
        return wrap(id.substr(0, dot)) + "." + wrap(id.substr(dot + 1));
    }

    // ---------------------------------------------------------------
    // Wrap a model-owned column reference. If `id` is already
    // qualified ("table.col"), behaves exactly like wrapQualified.
    // If it is bare ("col"), prefixes with the owning model's table
    // so that the emitted SQL disambiguates against columns pulled
    // in by a JOIN of the same name (e.g. `id` from both `games`
    // and `users`). Without this, `compileSelect` emits an unqualified
    // column list and JOIN-ed queries fail on same-named columns
    // ("Duplicate column name 'id'" on MySQL/SingleStore, ambiguous
    // reference on Postgres).
    // ---------------------------------------------------------------
    virtual string wrapModelColumn(const string& id, const string& modelTable) const
    {
        if (id.find('.') != string::npos) return wrapQualified(id);
        return wrap(modelTable) + "." + wrap(id);
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

    // Sanitize the logical connector between WHERE clauses. Only
    // "AND" and "OR" are permitted; anything else is a bug (or an
    // injection attempt if the value ever became caller-controlled).
    // Case-insensitive input, uppercased output.
    [[nodiscard]] static string sanitizeConnector(std::string_view c)
    {
        std::string upper;
        upper.reserve(c.size());
        std::transform(c.begin(), c.end(), std::back_inserter(upper),
                       [](unsigned char ch){ return std::toupper(ch); });
        if (upper == "AND" || upper == "OR") return upper;
        throw runtime_error("Grammar: rejected WHERE connector: '"
                            + std::string(c) + "'");
    }

    // Compile a WHERE clause using placeholders. Each value is appended
    // to `params`; the operator passes through the allowlist. The
    // logical connector on the second and subsequent clauses is taken
    // from `WhereClause::connector` (allowed: AND / OR) — the first
    // clause never emits a connector.
    virtual string compileWheres(const vector<Garvan::WhereClause>& wheres,
                                 vector<json>& params) const
    {
        if (wheres.empty()) return "";

        string sql = " WHERE ";

        for (size_t i = 0; i < wheres.size(); ++i)
        {
            const auto& w = wheres[i];
            const string op = sanitizeOperator(w.op);

            if (i != 0) {
                sql += " " + sanitizeConnector(w.connector) + " ";
            }

            // IN / NOT IN: each element in `values` becomes its own
            // placeholder — `col IN ($1, $2, ...)`. Populated by
            // Builder::whereIn / orWhereIn / whereNotIn.
            const bool isInOp = (op == "IN" || op == "NOT IN");
            if (isInOp && !w.values.empty()) {
                sql += wrapQualified(w.column) + " " + op + " (";
                for (size_t k = 0; k < w.values.size(); ++k) {
                    if (k) sql += ", ";
                    sql += placeholder(params.size());
                    params.push_back(json(w.values[k]));
                }
                sql += ")";
                continue;
            }

            // IS / IS NOT NULL: SQL semantics don't permit binding
            // NULL through a placeholder (`col IS $1` with $1='NULL'
            // compares against the string 'NULL', not the SQL NULL
            // literal). The Builder's nullptr_t overload marks such
            // values with the sentinel string "NULL"; recognise it
            // here and emit the literal inline, without a bind.
            const bool isNullOp = (op == "IS" || op == "IS NOT");
            const bool valueIsNull = (w.value == "NULL" || w.value == "null");
            if (isNullOp && valueIsNull) {
                sql += wrapQualified(w.column) + " " + op + " NULL";
            } else {
                sql += wrapQualified(w.column) + " " + op + " "
                     + placeholder(params.size());
                params.push_back(json(w.value));
            }
        }

        return sql;
    }

    // ---------------------------------------------------------------
    // Compile the SELECT projection list into `query`, respecting
    // priority: `selects` (Column/Aliased/Raw) wins; otherwise
    // withPrivate → "<table>.*"; otherwise the `columns` wrapped
    // list; empty falls back to "<table>.*".
    //
    // Parameters are extracted from Builder by the caller (Builder
    // is only forward-declared here — see grammar.h header order).
    //
    // Reused by every SQL grammar that emits a simple projection
    // (Postgres inner subquery, SingleStore, SQLite, MonetDB).
    // MySQL's JSON_OBJECT-based projection overrides compileSelect
    // entirely and throws on non-empty selects — mapping to literal
    // keys is not implemented.
    // ---------------------------------------------------------------
    virtual void compileProjection(
                const vector<Garvan::SelectClause>& selects,
                const vector<string>& columns,
                bool withPrivate,
                const string& table,
                string& query) const
    {
        if (!selects.empty()) {
            for (size_t i = 0; i < selects.size(); ++i) {
                const auto& s = selects[i];
                if (i) query += ", ";
                switch (s.kind) {
                    case Garvan::SelectKind::Column:
                        query += wrapModelColumn(s.expr, table);
                        break;
                    case Garvan::SelectKind::Aliased:
                        query += wrapModelColumn(s.expr, table)
                              + " AS " + wrap(s.alias);
                        break;
                    case Garvan::SelectKind::Raw:
                        // Verbatim SQL — caller is responsible for
                        // guarding the expression against injection.
                        query += s.expr;
                        if (!s.alias.empty())
                            query += " AS " + wrap(s.alias);
                        break;
                }
            }
            return;
        }
        if (withPrivate || columns.empty()) {
            query += wrap(table) + ".*";
            return;
        }
        for (size_t i = 0; i < columns.size(); ++i) {
            query += wrapModelColumn(columns[i], table);
            if (i != columns.size() - 1) query += ", ";
        }
    }

    // ---------------------------------------------------------------
    // Compile a list of JoinClause into ` <type> JOIN <table> ON ...`
    // fragments. Operator за ON clause се минава през същата allowlist
    // като WHERE-овете. За CROSS JOIN не се емитва ON.
    //
    // Емитираните идентификатори минават през wrapQualified, за да
    // работи както `users.id` / `orders.user_id`, така и голи
    // (`id`) — в който случай отговорността за уникалност пада на
    // caller-а.
    // ---------------------------------------------------------------
    virtual string compileJoins(const vector<Garvan::JoinClause>& joins) const
    {
        if (joins.empty()) return "";

        string sql;
        for (const auto& j : joins)
        {
            switch (j.type) {
                case Garvan::JoinClause::Type::Inner: sql += " INNER JOIN "; break;
                case Garvan::JoinClause::Type::Left:  sql += " LEFT JOIN ";  break;
                case Garvan::JoinClause::Type::Right: sql += " RIGHT JOIN "; break;
                case Garvan::JoinClause::Type::Cross: sql += " CROSS JOIN "; break;
            }
            sql += wrap(j.table);
            if (!j.alias.empty()) {
                sql += " AS " + wrap(j.alias);
            }

            if (j.type == Garvan::JoinClause::Type::Cross) continue;

            const string op = sanitizeOperator(j.op);
            sql += " ON " + wrapQualified(j.leftColumn) + " " + op + " "
                         + wrapQualified(j.rightColumn);
        }
        return sql;
    }

};

#endif // GARVAN_GRAMMAR_H
