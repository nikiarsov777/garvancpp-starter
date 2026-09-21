#ifndef GARVAN_BUILDER_H
#define GARVAN_BUILDER_H

#include <cctype>
#include <concepts>
#include <cstdio>
#include <exception>
#include <expected>
#include <initializer_list>
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

    // Explicit, fluent SQL-shaped JOIN-и (`join / leftJoin / rightJoin /
    // innerJoin / crossJoin`). Пази се отделно от `joinModel` (който
    // държи ORM-relations metadata) — двата пътя са независими и
    // могат да съществуват едновременно за една заявка.
    vector<JoinClause> joins;

    vector<string> public_columns;
    vector<string> private_columns;

    string order_by = "1 asc";

    bool isMany = false;

    json insertData;
    json updateData;

    // ---------------------------------------------------------------
    // RAW статe. Ако `rawSet` е `true`, `executeQuery / executeWrite`
    // подминават нормалната compile pipeline и делегират към
    // `executeRaw()`. `rawParams` пази вече ре-номерирани стойности
    // (positional подредбата на срещаните `?` / `:name`).
    //
    // За Mongo-envelope path (`rawJson`) — `rawIsJsonEnvelope = true`;
    // `rawSql` съдържа сериализирания JSON. Detection на грешна
    // backend употреба (SQL raw на Mongo или обратно) става в
    // `executeRaw()` през `Grammar::isSql()`.
    // ---------------------------------------------------------------
    bool         rawSet             = false;
    bool         rawIsJsonEnvelope  = false;
    std::string  rawSql;
    std::vector<json> rawParams;




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
        if (rawSet) return executeRaw();
        if (!grammar) {
            std::fprintf(stderr, "Builder::executeQuery: grammar is null\n");
            return json::Array();
        }
        PreparedStatement ps = grammar->compileSelect(*this);
        return dbClient->execute(ps);
    }

    [[nodiscard]] json executeWrite()
    {
        if (rawSet) return executeRaw();
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

    // ================================================================
    // RAW SQL / raw JSON envelope path.
    //
    //   builder->raw("SELECT * FROM users WHERE id = ?", {42})->get();
    //   builder->raw("... WHERE email = :email",
    //                json::Object{{"email", e}})->get();
    //   builder->rawJson({{"collection","users"}, {"filter", ...}})->get();
    //
    // Placeholder семантика:
    //   * положителни `?`  — консумират `params[i]` по ред;
    //   * named `:name`    — очаква `params.isObject()` с този ключ;
    //   * не се смесват    — throw при sql с и двата вида.
    //
    // Auto-detection SELECT vs write: `isReadStatement()` гледа
    // първата смислена дума (skip whitespace/comments/CTE `WITH`).
    // Терминалите (`get`, `first`, `executeWrite`) все още работят;
    // те delegate-ват към `executeRaw()` когато `hasRaw()` е `true`.
    // ================================================================

    Builder* raw(std::string_view sql, std::initializer_list<json> params)
    {
        return raw(sql, std::vector<json>(params.begin(), params.end()));
    }

    Builder* raw(std::string_view sql, std::vector<json> params = {})
    {
        assertNonEmptyRawSql(sql);
        rawSet = true;
        rawIsJsonEnvelope = false;
        rawSql = std::string(sql);
        rawParams = std::move(params);
        return this;
    }

    // Named placeholders (`:name`) — `params` трябва да е JSON object.
    Builder* raw(std::string_view sql, json namedParams)
    {
        assertNonEmptyRawSql(sql);
        if (!namedParams.isObject()) {
            throw std::runtime_error(
                "Builder::raw: named-params overload изисква JSON object");
        }
        rawSet = true;
        rawIsJsonEnvelope = false;
        rawSql = std::string(sql);
        // Копираме map-а в вътрешен буфер; final ordering идва при rewrite.
        rawParams.clear();
        rawParams.push_back(std::move(namedParams));  // sentinel: single json object
        return this;
    }

    // Mongo-side raw: envelope-ът е самият JSON, който `MongoConnection`
    // очаква (`{collection, filter, projection, ...}`).
    Builder* rawJson(json envelope)
    {
        rawSet = true;
        rawIsJsonEnvelope = true;
        // Сериализираме envelope-а към sql стринга — това е нормалният
        // транспорт към `MongoConnection::execute`.
        rawSql = envelope.dump();
        rawParams.clear();
        return this;
    }

    [[nodiscard]] bool hasRaw()  const { return rawSet; }
    [[nodiscard]] const std::string&        getRawSql()    const { return rawSql; }
    [[nodiscard]] const std::vector<json>&  getRawParams() const { return rawParams; }

    // Auto-детекция SELECT vs write. Използва се от `Model::raw` за
    // да реши дали да върне суров резултат от read path или write.
    [[nodiscard]] static bool isReadStatement(std::string_view sql) noexcept
    {
        size_t i = 0;
        auto skipWs = [&]() {
            while (i < sql.size()) {
                unsigned char c = static_cast<unsigned char>(sql[i]);
                if (std::isspace(c)) { ++i; continue; }
                if (c == '-' && i + 1 < sql.size() && sql[i+1] == '-') {
                    while (i < sql.size() && sql[i] != '\n') ++i;
                    continue;
                }
                if (c == '/' && i + 1 < sql.size() && sql[i+1] == '*') {
                    i += 2;
                    while (i + 1 < sql.size() && !(sql[i]=='*' && sql[i+1]=='/')) ++i;
                    if (i + 1 < sql.size()) i += 2;
                    continue;
                }
                break;
            }
        };
        skipWs();
        size_t start = i;
        while (i < sql.size() && std::isalpha(static_cast<unsigned char>(sql[i]))) ++i;
        std::string kw{sql.substr(start, i - start)};
        std::transform(kw.begin(), kw.end(), kw.begin(),
                       [](unsigned char c){ return std::toupper(c); });
        return kw == "SELECT" || kw == "WITH"   || kw == "SHOW"
            || kw == "EXPLAIN"|| kw == "PRAGMA" || kw == "VALUES"
            || kw == "TABLE"  || kw == "DESCRIBE" || kw == "DESC";
    }

    // Централно raw изпълнение. Проверява backend-семантиката,
    // прави placeholder rewriting (positional `?` → `$N` за
    // Postgres; named `:name` → positional по грамматика),
    // log-ва финалния SQL на stderr и извиква `dbClient->execute`.
    [[nodiscard]] json executeRaw()
    {
        if (!rawSet) {
            throw std::runtime_error("Builder::executeRaw: no raw statement set");
        }
        if (!grammar || !dbClient) {
            throw std::runtime_error("Builder::executeRaw: grammar/dbClient is null");
        }

        // Mongo path — envelope се предава без rewriting.
        if (rawIsJsonEnvelope) {
            if (grammar->isSql()) {
                throw std::runtime_error(
                    "Builder::rawJson: envelope не се поддържа на SQL backend; "
                    "ползвай raw(sql, params).");
            }
            std::fprintf(stderr, "[Garvan::rawJson] %s\n", rawSql.c_str());
            PreparedStatement ps{rawSql, {}};
            return dbClient->execute(ps);
        }

        // SQL path — блокира на Mongo backend.
        if (!grammar->isSql()) {
            throw std::runtime_error(
                "Builder::raw: SQL raw не се поддържа на Mongo backend; "
                "ползвай rawJson(envelope).");
        }

        auto [rewritten, orderedParams] = rewritePlaceholders(rawSql, rawParams, *grammar);
        std::fprintf(stderr, "[Garvan::raw] %s\n", rewritten.c_str());
        PreparedStatement ps{std::move(rewritten), std::move(orderedParams)};
        return dbClient->execute(ps);
    }

private:
    static void assertNonEmptyRawSql(std::string_view sql)
    {
        for (unsigned char c : sql) {
            if (!std::isspace(c)) return;
        }
        throw std::runtime_error("Builder::raw: празен SQL стринг");
    }

    // Rewrite-ва `?` / `:name` към backend placeholder-ите и
    // изгражда финалния positional param vector. Хвърля при:
    //   - смесване на positional и named,
    //   - broi ? != params.size(),
    //   - :name без ключ в JSON object-а,
    //   - празен sql.
    // Skip-ва `'...'` литерали (с `''` escape), `--` line comments
    // и `/* ... */` block comments — placeholder-ите вътре в тях
    // остават непроменени.
    static std::pair<std::string, std::vector<json>>
    rewritePlaceholders(const std::string& sql,
                        const std::vector<json>& inputParams,
                        const Grammar& g)
    {
        std::string out;
        out.reserve(sql.size() + 8);
        std::vector<json> ordered;

        // Detect режима: positional (не-object params) или named
        // (един json object). Празен `inputParams` = positional-0.
        bool named = false;
        const json* namedObj = nullptr;
        if (inputParams.size() == 1 && inputParams.front().isObject()) {
            named = true;
            namedObj = &inputParams.front();
        }

        size_t positionalIdx = 0;
        size_t i = 0;
        const size_t N = sql.size();
        bool sawPositional = false;
        bool sawNamed      = false;

        while (i < N) {
            char c = sql[i];

            // '...' string literal — пропускаме до затварящия '.
            if (c == '\'') {
                out.push_back(c);
                ++i;
                while (i < N) {
                    char d = sql[i];
                    out.push_back(d);
                    ++i;
                    if (d == '\'') {
                        // '' е escape вътре в стринг
                        if (i < N && sql[i] == '\'') {
                            out.push_back('\'');
                            ++i;
                            continue;
                        }
                        break;
                    }
                }
                continue;
            }

            // -- line comment
            if (c == '-' && i + 1 < N && sql[i+1] == '-') {
                while (i < N && sql[i] != '\n') { out.push_back(sql[i]); ++i; }
                continue;
            }

            // /* ... */ block comment
            if (c == '/' && i + 1 < N && sql[i+1] == '*') {
                out.push_back(sql[i]); out.push_back(sql[i+1]); i += 2;
                while (i + 1 < N && !(sql[i]=='*' && sql[i+1]=='/')) {
                    out.push_back(sql[i]); ++i;
                }
                if (i + 1 < N) { out.push_back(sql[i]); out.push_back(sql[i+1]); i += 2; }
                continue;
            }

            // Positional ?
            if (c == '?') {
                sawPositional = true;
                if (sawNamed) {
                    throw std::runtime_error(
                        "Builder::raw: смесване на `?` и `:name` "
                        "placeholder-и в един SQL не е позволено.");
                }
                if (named) {
                    throw std::runtime_error(
                        "Builder::raw: `?` в SQL, а params е JSON object "
                        "(очакваше се vector).");
                }
                if (positionalIdx >= inputParams.size()) {
                    throw std::runtime_error(
                        "Builder::raw: повече `?` отколкото стойности в params.");
                }
                out += g.placeholder(ordered.size());
                ordered.push_back(inputParams[positionalIdx]);
                ++positionalIdx;
                ++i;
                continue;
            }

            // Named :name  (:: е Postgres cast — пропускаме)
            if (c == ':' && i + 1 < N && sql[i+1] != ':') {
                unsigned char nx = static_cast<unsigned char>(sql[i+1]);
                if (std::isalpha(nx) || nx == '_') {
                    sawNamed = true;
                    if (sawPositional) {
                        throw std::runtime_error(
                            "Builder::raw: смесване на `?` и `:name` "
                            "placeholder-и в един SQL не е позволено.");
                    }
                    if (!named) {
                        throw std::runtime_error(
                            "Builder::raw: `:name` изисква named-params "
                            "overload (JSON object).");
                    }
                    ++i;  // skip ':'
                    size_t nameStart = i;
                    while (i < N) {
                        unsigned char x = static_cast<unsigned char>(sql[i]);
                        if (std::isalnum(x) || x == '_') { ++i; continue; }
                        break;
                    }
                    std::string name = sql.substr(nameStart, i - nameStart);
                    if (namedObj->asObject().count(name) == 0) {
                        throw std::runtime_error(
                            "Builder::raw: липсва named param `:" + name + "`");
                    }
                    out += g.placeholder(ordered.size());
                    ordered.push_back((*namedObj)[name]);
                    continue;
                }
            }

            // Postgres :: cast — оставяме двете двоеточия непроменени
            if (c == ':' && i + 1 < N && sql[i+1] == ':') {
                out.push_back(':'); out.push_back(':'); i += 2; continue;
            }

            out.push_back(c);
            ++i;
        }

        if (!named && positionalIdx != inputParams.size()) {
            throw std::runtime_error(
                "Builder::raw: по-малко `?` отколкото стойности в params.");
        }
        return {std::move(out), std::move(ordered)};
    }

public:

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

    // ================================================================
    // JOINS — ниско-нивов fluent API, независим от `joinModel`
    // (relations). Емисията се прави от `Grammar::compileJoins`, което
    // сега се извиква от всеки SQL backend в `compileSelect`.
    //
    // Употреба:
    //     builder->join("orders", "users.id", "=", "orders.user_id")
    //            ->where("users.active", "=", true)
    //            ->get();
    //
    // За CROSS JOIN колоните се игнорират (ON clause не се емитва).
    // За Mongo backend се емитира `$lookup`-подобна метадата в JSON
    // envelope-а — виж `MongoGrammar::compileSelect`.
    // ================================================================
    Builder* join(std::string_view table,
                  std::string_view left,
                  std::string_view op,
                  std::string_view right)
    {
        joins.push_back({JoinClause::Type::Inner,
                         std::string(table), std::string(left),
                         std::string(op),    std::string(right)});
        return this;
    }

    // Двуаргументен overload — приема "=", шорткът е чест.
    Builder* join(std::string_view table,
                  std::string_view left,
                  std::string_view right)
    {
        return join(table, left, "=", right);
    }

    Builder* innerJoin(std::string_view table,
                       std::string_view left,
                       std::string_view op,
                       std::string_view right)
    {
        return join(table, left, op, right);
    }

    Builder* leftJoin(std::string_view table,
                      std::string_view left,
                      std::string_view op,
                      std::string_view right)
    {
        joins.push_back({JoinClause::Type::Left,
                         std::string(table), std::string(left),
                         std::string(op),    std::string(right)});
        return this;
    }

    Builder* leftJoin(std::string_view table,
                      std::string_view left,
                      std::string_view right)
    {
        return leftJoin(table, left, "=", right);
    }

    Builder* rightJoin(std::string_view table,
                       std::string_view left,
                       std::string_view op,
                       std::string_view right)
    {
        joins.push_back({JoinClause::Type::Right,
                         std::string(table), std::string(left),
                         std::string(op),    std::string(right)});
        return this;
    }

    Builder* rightJoin(std::string_view table,
                       std::string_view left,
                       std::string_view right)
    {
        return rightJoin(table, left, "=", right);
    }

    Builder* crossJoin(std::string_view table)
    {
        joins.push_back({JoinClause::Type::Cross,
                         std::string(table), "", "=", ""});
        return this;
    }

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
    [[nodiscard]] const vector<JoinClause>  &getJoins()  const { return joins; }
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
        joins.clear();
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
