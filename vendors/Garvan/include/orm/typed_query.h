#ifndef GARVAN_TYPED_QUERY_H
#define GARVAN_TYPED_QUERY_H
#pragma once

// ---------------------------------------------------------------
// TypedQuery<T> -- fluent, typed pipeline over `Garvan::Model`.
//
// The classical Garvan chain (`Model::where<T>(...)->first()`) is
// preserved for back-compat and continues to return `json`.  The
// typed pipeline lives alongside it:
//
//     User user = User::query<User>()
//                   ->where("email", email)
//                   ->where("active","=", true)
//                   ->firstOrFail();     // -> User (throws if 0 rows)
//
//     auto opt = User::query<User>()->find(id);   // std::optional<User>
//     std::vector<User> list = User::query<User>()
//                                ->where("active","=", true)
//                                ->get();
//
// Bulk mutations require an explicit WHERE chain (see the guards
// in `update()` / `remove()`) so an accidental empty-chain call
// cannot wipe or rewrite a whole table.
//
// See `GARVAN.md` -- resolves gaps 1 (Model::where IS NULL),
// 2 (DELETE), 6 (non-PK UPDATE) and 7 (rehydration).
// ---------------------------------------------------------------

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "builder.h"
#include "../tools/JsonValue.h"

namespace Garvan {

class Model;   // full definition arrives via model.h before the
               // template methods below are ever instantiated.

template <typename T>
class TypedQuery {
public:
    TypedQuery()
        : instance(std::make_unique<T>())
    {
        // Reuse the model's own `init()` -- guarantees a fresh
        // builder wired to the correct grammar / columns.
        instance->init();
    }

    // -----------------------------------------------------------
    // Chain composition. All overloads mirror the ones on
    // `Garvan::Model` (which itself forwards to `Builder`), so
    // existing WHERE muscle memory carries over 1:1.
    // -----------------------------------------------------------

    TypedQuery* where(std::string field, std::string value) {
        instance->where(std::move(field), std::move(value));
        return this;
    }

    TypedQuery* where(std::string field, std::string op, std::string value) {
        instance->where(std::move(field), std::move(op), std::move(value));
        return this;
    }

    // IS NULL / IS NOT NULL -- resolves GARVAN.md Gap 1 on the
    // typed surface. Forwards to the Builder overload directly
    // because Model has no nullptr overload yet.
    TypedQuery* where(std::string_view field, std::string_view op, std::nullptr_t) {
        instance->getBuilder()->where(field, op, nullptr);
        return this;
    }

    template <typename V>
    TypedQuery* where(std::string_view field, std::string_view op, V value) {
        instance->getBuilder()->where(field, op, value);
        return this;
    }

    TypedQuery* limit(int n) {
        instance->limit(n);
        return this;
    }

    // Idempotent no-op: the pipeline is already typed. Kept for
    // symmetry with an eventual untyped `Query::toModel<T>()`
    // switch and to make the "toModel()" step visible in call
    // sites that want to emphasise the typed intent.
    TypedQuery* toModel() { return this; }

    // -----------------------------------------------------------
    // Terminals.
    //   first()        -> std::optional<T>       (nullopt if 0 rows)
    //   firstOrFail()  -> T                      (throws)
    //   find(id)       -> std::optional<T>
    //   findOrFail(id) -> T
    //   get()          -> std::vector<T>
    // -----------------------------------------------------------

    std::optional<T> first() {
        auto* b = instance->getBuilder();
        b->limit = 1;
        return firstFromRawResult(b->get());
    }

    T firstOrFail() {
        auto opt = first();
        if (!opt) throw std::runtime_error("TypedQuery::firstOrFail: no rows");
        return std::move(*opt);
    }

    std::optional<T> find(int id) {
        instance->getBuilder()->where("id", "=", std::to_string(id));
        return first();
    }

    T findOrFail(int id) {
        auto opt = find(id);
        if (!opt) throw std::runtime_error(
            "TypedQuery::findOrFail: id " + std::to_string(id) + " not found");
        return std::move(*opt);
    }

    std::vector<T> get() {
        JsonValue raw = instance->getBuilder()->get();
        JsonValue parsed = parseResult(raw);
        std::vector<T> out;
        if (!parsed.isArray()) return out;
        out.reserve(parsed.size());
        for (const auto& row : parsed.asArray()) {
            if (!row.isObject()) continue;
            T item;
            item.init();
            item.hydrate(row);
            out.push_back(std::move(item));
        }
        return out;
    }

    // -----------------------------------------------------------
    // Bulk mutations. Empty-WHERE calls throw -- callers that
    // really want a table-wide sweep must construct one on their
    // own (or add an explicit `updateAll()` in a future revision).
    // -----------------------------------------------------------

    void update(const JsonValue& fields) {
        auto* b = instance->getBuilder();
        if (b->getWheres().empty()) {
            throw std::runtime_error(
                "TypedQuery::update refuses to run without a where(...) "
                "chain. Add a filter or use a raw statement.");
        }
        (void)b->update(fields)->executeWrite();
    }

    void remove() {
        auto* b = instance->getBuilder();
        if (b->getWheres().empty()) {
            throw std::runtime_error(
                "TypedQuery::remove refuses to run without a where(...) "
                "chain. Add a filter or use a raw statement.");
        }
        // executeWrite() falls through to compileDelete when both
        // insertData and updateData are empty (see builder.h).
        (void)b->executeWrite();
    }

private:
    std::unique_ptr<T> instance;

    // The connection layer stores results as `RawJson` (a JSON
    // string, see e.g. sqlite_connection.cpp). Convert it back to
    // a walkable Object/Array tree via JsonValue::parse.
    static JsonValue parseResult(const JsonValue& raw) {
        // dump() on a RawJson returns the raw string as-is; on any
        // other kind it re-serialises. Either way we end up with a
        // parseable JSON string.
        std::string s = raw.dump();
        if (s.empty() || s == "null") {
            return JsonValue::parse("[]");
        }
        return JsonValue::parse(s);
    }

    std::optional<T> firstFromRawResult(const JsonValue& raw) {
        JsonValue parsed = parseResult(raw);
        const JsonValue* row = nullptr;
        if (parsed.isArray()) {
            if (parsed.size() == 0) return std::nullopt;
            row = &parsed.asArray().front();
        } else if (parsed.isObject()) {
            row = &parsed;
        } else {
            return std::nullopt;
        }
        if (!row->isObject() || row->size() == 0) return std::nullopt;
        T item;
        item.init();
        item.hydrate(*row);
        return std::optional<T>(std::move(item));
    }
};

}   // namespace Garvan

// -----------------------------------------------------------------
// Out-of-line template definitions for the Model statics declared
// in model.h. Defined here because they need the complete
// TypedQuery<T> type.
// -----------------------------------------------------------------
namespace Garvan {

template <ModelType T>
inline std::unique_ptr<TypedQuery<T>> Model::query() {
    return std::make_unique<TypedQuery<T>>();
}

template <ModelType T>
inline T Model::findAs(int id) {
    return query<T>()->findOrFail(id);
}

template <ModelType T>
inline std::optional<T> Model::tryFindAs(int id) {
    return query<T>()->find(id);
}

}   // namespace Garvan

#endif // GARVAN_TYPED_QUERY_H
