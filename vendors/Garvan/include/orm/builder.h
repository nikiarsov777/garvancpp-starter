#ifndef GARVAN_BUILDER_H
#define GARVAN_BUILDER_H

#include <concepts>
#include <cstdio>
#include <exception>
#include <expected>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>
#include "omodel.h"
#include "connection/db_connection.h"
#include "grammar/grammar.h"

namespace Garvan {

// ---------------------------------------------------------------
// DbError — типизирана грешка за новите `try*()` API варианти.
// Старите методи (`get`, `first`, `find`, `firstOrFail`, ...)
// продължават да хвърлят `std::runtime_error` — те не са
// пренаписани, за да не се чупи BC.
// ---------------------------------------------------------------
struct DbError {
    enum class Code {
        Connection,   // connection / auth / network
        Syntax,       // грешен SQL, unknown identifier
        Constraint,   // NOT NULL / UNIQUE / FK / CHECK
        NotFound,     // firstOrFail / findOrFail miss
        Unknown       // всичко останало
    };
    Code        code{Code::Unknown};
    std::string message;
    std::string sqlstate;   // при налична
};

} // namespace Garvan

using namespace std;

namespace Garvan
{

class Builder
{
    friend class ORM::OModel;

private:
    DbClient *dbClient;
    Grammar *grammar;

    string table;
    string primaryKey = "id";

    vector<WhereClause> wheres;

    vector<pair<string, ORM::OModel>> joinModel;

    vector<string> public_columns;
    vector<string> private_columns;

    string order_by = "1 asc";

    bool isMany = false;

    json insertData;
    json updateData;




public:
    int limit = 10;
    int offset = 0;

    [[nodiscard]] const json& getInsertData() const {
        return insertData;
    }

    [[nodiscard]] const json& getUpdateData() const {
        return updateData;
    }

    Builder(DbClient *dbClient,
            Grammar *grammar,
            string table,
            vector<string> public_columns,
            vector<string> private_columns)
    {
        this->dbClient = dbClient;
        this->grammar = grammar;
        this->table = table;
        this->public_columns = public_columns;
        this->private_columns = private_columns;
    }

    // централен execution — uses the parameterized PreparedStatement
    // path so values flow through native binding, not SQL string concat.
    [[nodiscard]] json executeQuery()
    {
        if (!grammar) {
            std::fprintf(stderr, "Builder::executeQuery: grammar is null\n");
            return json::Array();
        }
        PreparedStatement ps = grammar->compileSelect(*this);
        return dbClient->execute(ps);
    }

    [[nodiscard]] json executeWrite()
    {
        if (!grammar) {
            std::fprintf(stderr, "Builder::executeWrite: grammar is null\n");
            return json::Object();
        }

        PreparedStatement ps;

        if (!insertData.empty())
            ps = grammar->compileInsert(*this);
        else if (!updateData.empty())
            ps = grammar->compileUpdate(*this);
        else
            ps = grammar->compileDelete(*this);

        return dbClient->execute(ps);
    }

    // =======================
    // SELECT
    // =======================

    [[nodiscard]] json get()
    {
        return executeQuery();
    }

    [[nodiscard]] json get(vector<string> selectedColumns)
    {
        this->public_columns = std::move(selectedColumns);
        return get();
    }

    [[nodiscard]] json first()
    {
        this->limit = 1;
        return executeQuery();
    }

    [[nodiscard]] json firstOrFail()
    {
        auto result = first();
        if (result.empty())
            throw runtime_error("Record not found");
        return result;
    }

    [[nodiscard]] json find(int id)
    {
        where(primaryKey, "=", std::to_string(id));
        return first();
    }

    [[nodiscard]] json findOrFail(int id)
    {
        auto result = find(id);
        if (result.empty())
            throw runtime_error("Record not found");
        return result;
    }

    // =======================
    // WHERE
    // =======================
    //
    // Основните overloads вече приемат `string_view` — премахва
    // излишен string copy при повикване с литерал или `.c_str()`.
    // Старите `string` overloads са оставени за BC (те просто
    // делегират на новите чрез неявна конверсия).

    Builder *where(std::string_view field, std::string_view value)
    {
        wheres.push_back({std::string(field), "=", std::string(value)});
        return this;
    }

    Builder *where(std::string_view field, std::string_view op, std::string_view value)
    {
        wheres.push_back({std::string(field), std::string(op), std::string(value)});
        return this;
    }

    // ----------------------------------------------------------------
    // Typed WHERE overloads — приемат numeric/bool/nullptr стойности
    // и вътрешно ги нормализират към string, за да пазят пълна BC
    // със стария `PreparedStatement { sql, vector<json> params }`
    // pipeline. За консумиращия код това означава:
    //     builder->where("id", "=", 42);
    //     builder->where("active", "=", true);
    //     builder->where("deleted_at", "IS", nullptr);
    // Пълно typed binding (variant → native driver bind) е бъдещо
    // разширение — изисква пренаписване на всеки *_connection.cpp.
    // ----------------------------------------------------------------
    template <typename V>
        requires (std::integral<V> && !std::same_as<std::remove_cvref_t<V>, bool>)
              || std::floating_point<V>
    Builder* where(std::string_view field, std::string_view op, V value)
    {
        wheres.push_back({std::string(field), std::string(op), std::to_string(value)});
        return this;
    }

    Builder* where(std::string_view field, std::string_view op, bool value)
    {
        wheres.push_back({std::string(field), std::string(op),
                          value ? std::string("true") : std::string("false")});
        return this;
    }

    Builder* where(std::string_view field, std::string_view op, std::nullptr_t)
    {
        wheres.push_back({std::string(field), std::string(op), std::string("NULL")});
        return this;
    }

    // ----------------------------------------------------------------
    // Value-chain overloads (C++23 deducing `this`).
    //
    // Позволяват chain върху value/references, без задължителния
    // pointer-hop от classical fluent API:
    //     Builder b(...);
    //     b.whereRef("id","1").whereRef("age",">","18").get();
    //
    // Върнатият тип е `Self&` (същият cv-/ref-qualified тип на
    // приемника), така че методът работи еднакво добре за `Builder`
    // и за евентуален бъдещ подклас. Не заместват pointer chain-a;
    // старите `where(...)*` остават за BC.
    // ----------------------------------------------------------------
    template <typename Self>
    auto&& whereRef(this Self&& self, std::string_view field, std::string_view value)
    {
        self.wheres.push_back({std::string(field), "=", std::string(value)});
        return std::forward<Self>(self);
    }

    template <typename Self>
    auto&& whereRef(this Self&& self, std::string_view field,
                                       std::string_view op, std::string_view value)
    {
        self.wheres.push_back({std::string(field), std::string(op), std::string(value)});
        return std::forward<Self>(self);
    }

    // =======================
    // RELATIONS (оставени както са)
    // =======================

    Builder *hasOne(ORM::OModel model, string fKey = "", string lKey = "")
    {
        joinModel.push_back({fKey, model});
        return this;
    }

    Builder *hasMany(ORM::OModel model, string fKey = "", string ownerKey = "")
    {
        isMany = true;
        joinModel.push_back({fKey, model});
        return this;
    }

    Builder *belongsTo(ORM::OModel model, string fKey = "", string ownerKey = "")
    {
        joinModel.push_back({fKey, model});
        return this;
    }

    Builder *belongsToMany(ORM::OModel model, string table = "", string fKey = "", string ownerKey = "")
    {
        joinModel.push_back({fKey, model});
        return this;
    }

    Builder* insert(const json& data)
    {
        insertData = data;
        return this;
    }

    Builder* update(const json& data)
    {
        updateData = data;
        return this;
    }

    Builder* remove()
    {
        return this;
    }

    // =======================
    // GETTERS (за Grammar)
    // =======================

    [[nodiscard]] const string &getTable() const { return table; }
    [[nodiscard]] const vector<WhereClause> &getWheres() const { return wheres; }
    [[nodiscard]] const string &getOrder() const { return order_by; }
    [[nodiscard]] int getLimit() const { return limit; }
    [[nodiscard]] int getOffset() const { return offset; }
    [[nodiscard]] const vector<string> &getColumns() const { return public_columns; }

    // =======================
    // RESET
    // =======================

    void reset()
    {
        wheres.clear();
        limit = 10;
        offset = 0;
    }

    // ================================================================
    // Non-throwing `try*` варианти (C++23 `std::expected`).
    //
    // Хващат exception-a от долните слоеве, класифицират го грубо
    // (Syntax vs Connection vs Unknown според съдържанието на
    // `what()`) и връщат `unexpected(DbError{...})`. Разработчикът
    // избира стил:
    //     json res = user->where("id","1")->first();          // throws
    //     auto res = user->where("id","1")->tryFirst();        // expected
    //     if (!res) log(res.error().message);
    //
    // Класификация: минимална евристика. За прецизна класификация
    // ще ни трябват SQLSTATE-и от драйверите — оставено за бъдещо
    // разширение.
    // ================================================================

    [[nodiscard]] std::expected<json, DbError> tryGet() noexcept
    {
        try { return executeQuery(); }
        catch (const std::exception& e) { return std::unexpected(classify(e.what())); }
        catch (...)                     { return std::unexpected(DbError{DbError::Code::Unknown, "unknown", ""}); }
    }

    [[nodiscard]] std::expected<json, DbError> tryFirst() noexcept
    {
        try { limit = 1; return executeQuery(); }
        catch (const std::exception& e) { return std::unexpected(classify(e.what())); }
        catch (...)                     { return std::unexpected(DbError{DbError::Code::Unknown, "unknown", ""}); }
    }

    [[nodiscard]] std::expected<json, DbError> tryFind(int id) noexcept
    {
        try {
            where(primaryKey, "=", std::to_string(id));
            limit = 1;
            json result = executeQuery();
            if (result.empty())
                return std::unexpected(DbError{DbError::Code::NotFound, "record not found", ""});
            return result;
        }
        catch (const std::exception& e) { return std::unexpected(classify(e.what())); }
        catch (...)                     { return std::unexpected(DbError{DbError::Code::Unknown, "unknown", ""}); }
    }

    [[nodiscard]] std::expected<json, DbError> tryExecuteWrite() noexcept
    {
        try { return executeWrite(); }
        catch (const std::exception& e) { return std::unexpected(classify(e.what())); }
        catch (...)                     { return std::unexpected(DbError{DbError::Code::Unknown, "unknown", ""}); }
    }

private:
    // Прибрана евристика — конкретните SQLSTATE / driver кодове ще
    // се добавят в бъдеще с промени в connection layer-а.
    [[nodiscard]] static DbError classify(std::string_view what) noexcept
    {
        auto contains = [&](std::string_view needle) {
            return what.find(needle) != std::string_view::npos;
        };
        if (contains("syntax") || contains("Syntax"))
            return DbError{DbError::Code::Syntax, std::string(what), ""};
        if (contains("connect") || contains("Connect") || contains("network"))
            return DbError{DbError::Code::Connection, std::string(what), ""};
        if (contains("constraint") || contains("duplicate") || contains("unique"))
            return DbError{DbError::Code::Constraint, std::string(what), ""};
        if (contains("not found") || contains("Record not found"))
            return DbError{DbError::Code::NotFound, std::string(what), ""};
        return DbError{DbError::Code::Unknown, std::string(what), ""};
    }
};
}

#endif // GARVAN_BUILDER_H
