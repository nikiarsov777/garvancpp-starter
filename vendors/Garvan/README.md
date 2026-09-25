# garvan_vendor

C++23 vendor библиотека за Garvan framework: ORM, миграции, Crow HTTP,
plus `garvan-migrate` и `kalpasan` CLI-та.

## Build

```bash
./make.sh              # incremental build + sync към ../garvancpp-starter/vendors/Garvan/
./make.sh clean        # пълен rebuild
JOBS=8 ./make.sh       # override parallel jobs (default: 4)
```

`make.sh` е thin wrapper над CMake. **Никога** не подавай bare `-j` на
`cmake --build` — при Makefile generator това става `make -j` =
неограничен брой jobs → swap thrashing. Скриптът винаги минава
конкретен `-j "$JOBS"`.

Изисквания: GCC 14+ или Clang 18+ (за `deducing this` и `std::expected`),
CMake 3.16+.

## C++23 API

От версия C++23 rewrite ORM-ът излага паралелни, по-безопасни API-та
до старите (пълна BC е запазена). Ключови промени:

### 1. Concepts за static factory-та

```cpp
template <ModelType T> static T* where(std::string field, std::string value);
```

`ModelType` = `std::derived_from<T, Garvan::Model> && std::is_default_constructible_v<T>`.
Грешките при неправилен `T` са къси и четими.

### 2. Премахнат `delete this`

Терминалните методи (`get`, `find`, `first`, ...) вече не правят
`delete this`. Стек-инстанциите вече не са UB. Ownership на моделите
върнати от static `where<T>()` се държи в `thread_local` scratchpad
и се почиства при следващ `where<T>()` на същия thread или чрез
явен `Garvan::Model::flushScratch()`.

### 3. Typed WHERE overloads

```cpp
builder->where("id", "=", 42);           // int → "42"
builder->where("active", "=", true);     // bool → "true"
builder->where("deleted_at", "IS", nullptr);  // → "NULL"
```

Concept-guarded overloads за `integral`/`floating_point`/`bool`/`nullptr_t`.
Съществуващият string API остава непокътнат.

### 4. Deducing-`this` fluent API

```cpp
Builder b(...);
b.whereRef("id", "1").whereRef("age", ">", "18").get();
```

`whereRef` връща `Self&` вместо `Self*`, което позволява value chain-и.
Старите `Builder* where(...)` са пазени за BC.

### 5. `std::expected<json, DbError>` вместо exception

```cpp
auto result = user->tryFirst();
if (!result) {
    log(result.error().message);
    return;
}
json data = *result;
```

`DbError::Code` = `{ Connection, Syntax, Constraint, NotFound, Unknown }`.
Класификацията сега е евристична (по `what()`); прецизна класификация
чрез SQLSTATE идва в бъдещо разширение на connection layer-а. Старите
throwing методи (`get`, `first`, `find*`) са пазени за BC.

### 6. `TypedQuery<T>` — typed model pipeline (2026-08-21)

Класическият `Model::where<T>(...)->first()` връща `json`. Новият
typed pipeline връща реални model instance-и, с автоматична
рехидратация (`Model::hydrate()` попълва `attributes` + `id`) и
пълен CRUD цикъл върху заредения обект:

```cpp
// Chain по non-PK, mutate, save (UPDATE)
User u = User::query<User>()
           ->where("email", email)
           ->where("active","=", true)
           ->firstOrFail();
u.set("last_login", now);
u.save();                          // UPDATE ... WHERE id=<hydrated>

// By-id
User u = User::findAs<User>(id);                     // throws ако липсва
auto  opt = User::tryFindAs<User>(id);               // std::optional<User>

// Instance DELETE
User u = User::findAs<User>(id);
u.remove();                        // DELETE ... WHERE id=<hydrated>

// Bulk UPDATE / DELETE (empty WHERE → throws)

// Преход между състояния (state machine на cohort)
Invoice::query<Invoice>()->where("status","=","pending")
                        ->update({{"status","sent"},
                                  {"sent_at", now}});

// Логическо изтриване на съгласуван cohort
User::query<User>()->where("banned","=", true)
                   ->update({{"deleted_at", now}});          // soft-delete

// Физическо изтриване на изтекли редове
FcmToken::query<FcmToken>()->where("expires_at","<", now)
                           ->remove();                        // hard delete

// IS NULL на typed surface
Settings s = Settings::query<Settings>()
               ->where("user_id","IS", nullptr)
               ->firstOrFail();

// List rehydration
std::vector<User> active = User::query<User>()
                             ->where("active","=", true)
                             ->get();
```

**Terminal API на `TypedQuery<T>`:**

| Метод                | Return                     | Not-found |
| -------------------- | -------------------------- | --------- |
| `first()`            | `std::optional<T>`         | `nullopt` |
| `firstOrFail()`      | `T`                        | throws    |
| `find(int id)`       | `std::optional<T>`         | `nullopt` |
| `findOrFail(int id)` | `T`                        | throws    |
| `get()`              | `std::vector<T>`           | празен    |
| `update(json)`       | void                       | throws при празен WHERE |
| `remove()`           | void                       | throws при празен WHERE |
| `toModel()`          | `TypedQuery<T>*` (no-op)   | —         |

**Механика на rehydration.** Connection layer-ите пазят резултата
като `JsonValue::RawJson` (JSON string). `TypedQuery` го парсва през
новия `JsonValue::parse(std::string_view)` (RFC 8259, dependency-free)
и извиква `Model::hydrate(row)` за всеки Object в масива. Двойната
сериализация от предния workaround (`.dump()` + `crow::json::load`)
отпада.

**Затворени gap-ове от `GARVAN.md`:** Gap 1 (Model IS NULL — на typed
surface), Gap 2 (DELETE — instance + bulk), Gap 6 (UPDATE с non-PK
WHERE), Gap 7 (rehydration в model instance).

**Guards.** `TypedQuery<T>::update()` / `remove()` без предходен
`where(...)` chain хвърлят `std::runtime_error` (защита срещу
случайно full-table wipe). `Model::remove()` върху нехидриран
модел (празен id) също хвърля.

**BC.** Съществуващите `Model::where<T>()`, `first()`, `find(int)`,
`get()` (връщащи `json`) остават непроменени. Setter-ите остават
manual (`Model::set("email", v)`) — генерирани getter/setter-и не
се въвеждат в тази ревизия.

Виж `GARVAN.md` и `orm/typed_query.h` за пълния API surface.

### 7. SQL JOINs (2026-09-21)

Fluent, backend-portable JOIN API върху `Builder`, `Model` и
`TypedQuery<T>`. Затваря `GARVAN.md` **Gap 4** на explicit surface;
relation-driven compile (`hasOne` / `hasMany` / `belongsTo` / `with`)
все още попълва `Builder::joinModel` без емисия — отделен follow-up.

**API (одинаков на трите нива):**

```cpp
join     (table, left, op, right)  |  join     (table, left, right)   // op = "="
innerJoin(table, left, op, right)                                       // alias на join
leftJoin (table, left, op, right)  |  leftJoin (table, left, right)
rightJoin(table, left, op, right)  |  rightJoin(table, left, right)
crossJoin(table)                                                        // без ON
```

**Пример (typed chain):**

```cpp
auto rows = User::query<User>()
    ->leftJoin("orders", "users.id", "orders.user_id")
    ->where("users.active", "=", true)
    ->get();
```

**Механика.**

- Нов `Garvan::JoinClause { type, table, leftColumn, op, rightColumn }`
  се пази отделно от съществуващия `joinModel` (relations).
- `Grammar::compileJoins()` емитира ` INNER|LEFT|RIGHT|CROSS JOIN ...`
  фрагменти. Извиква се от `compileSelect` на Postgres, MySQL, SQLite,
  MonetDB и SingleStore grammars (SingleStore = MySQL wire protocol,
  затова наследява placeholder-и и quoting-а).
- Нов `Grammar::wrapQualified(id)` разцепва `"table.column"` и quotes
  двете страни независимо (`"users"."id"`). Приложен в SELECT колоните
  и в `compileWheres` — тоест `where("users.active", "=", true)` и
  `get({"users.id", "orders.total"})` работят без ръчна манипулация.
- Operator в `ON` clause минава през същия allowlist като WHERE.
- CROSS JOIN не емитира `ON` (колоните се игнорират).

**Mongo.** `MongoGrammar::compileSelect` не емитва SQL; вместо това
изхвърля `joins[]` масив в JSON envelope-а (`from` / `localField` /
`foreignField` / `type` за всеки join) — `MongoConnection` е очаквано
да го преведе към `$lookup` aggregation stages. `RIGHT JOIN` върху
Mongo хвърля `runtime_error` (няма `$lookup` еквивалент); `INNER` и
`LEFT` работят; `CROSS` се приема, но без match условие.

**Отворен sub-gap.** Релациите (`hasOne`, `hasMany`, `belongsTo`,
`belongsToMany`, `with`) продължават да push-ват в `Builder::joinModel`,
който не е свързан към `compileSelect`. Consumer-и, разчитащи на
eager loading, трябва да ползват експлицитен `leftJoin(...)`
докато този path не бъде довършен.

### 8. Raw SQL / JSON envelope (2026-09-21)

Escape hatch за случаи, които не се вписват в query builder-а:
сложни агрегати, CTEs, DDL, backend-specific извиквания. Пълен
portable placeholder rewriter — potребителят пише **един и същ**
SQL и работи и на Postgres (`$N`), и на SQLite/MySQL/MonetDB/SingleStore
(`?`).

**API — три нива.**

```cpp
// Static (Model)
JsonValue Model::raw(std::string_view sql, std::vector<JsonValue> params = {});
JsonValue Model::raw(std::string_view sql, JsonValue namedParams);   // {":name"}
JsonValue Model::rawJson(JsonValue envelope);                         // Mongo
template <ModelType T>
std::vector<T> Model::rawAs(std::string_view sql, std::vector<JsonValue> params = {});
template <ModelType T>
std::vector<T> Model::rawAs(std::string_view sql, JsonValue namedParams);

// Typed chain
TypedQuery<T>* raw(sql, params)  |  raw(sql, namedParams)  |  rawJson(envelope)

// Low-level Builder
Builder* raw(sql, params)  |  raw(sql, namedParams)  |  rawJson(envelope)
[[nodiscard]] json executeRaw();
```

Терминалите (`get / first / firstOrFail / find`) на `TypedQuery<T>`
работят непроменени — те short-circuit-ват към `executeRaw()` когато
има активен raw и продължават да hidrate-ват в `T` през `Model::hydrate`.

**Placeholder модел.**

- **Positional `?`** — консумира `params[i]` по ред на срещане.
- **Named `:name`** — очаква `params` да е JSON object; rewriter-ът
  преобразува към positional по реда на срещане.
- **Не се смесват** — SQL с и `?`, и `:name` хвърля `runtime_error`.
- Rewriter-ът извиква `Grammar::placeholder(i)` — Postgres замества
  към `$1..$N`, останалите backend-и оставят `?`.

**Skip zones** (rewriter-ът не пипа placeholder-и вътре в тях):
`'...'` string литерали (с `''` escape), `--` line comments,
`/* ... */` block comments, Postgres `::` cast operator.

**Guards** (всички хвърлят `std::runtime_error`):

- празен SQL,
- broj `?` ≠ params.size(),
- `:name` без съответен ключ в JSON object-а,
- `?` в SQL, но `params` е JSON object (или обратно),
- `raw(sql,...)` на Mongo backend → съобщение да се ползва `rawJson`,
- `rawJson(envelope)` на SQL backend → съобщение да се ползва `raw`.

Backend detection става през нов `Grammar::isSql()` — SQL backends
връщат `true`, `MongoGrammar` overrides към `false`.

**Auto-detection SELECT vs write.** `Builder::isReadStatement()`
разпознава `SELECT / WITH / SHOW / EXPLAIN / PRAGMA / VALUES / TABLE
/ DESCRIBE / DESC` (skip-ва leading whitespace / comments). Позволява
единен `raw()` name — не са нужни отделни `rawSelect` / `rawExec`.

**Log-ване.** `executeRaw()` печата `[Garvan::raw] <финален SQL>` на
stderr за всяка raw заявка (без параметрите — те могат да съдържат
sensitive данни). За `rawJson` — `[Garvan::rawJson] <envelope>`.

**Примери.**

```cpp
// Positional
json rows = Model::raw(
    "SELECT * FROM users WHERE created_at > ? AND active = ?",
    { since, true });

// Named
json rows = Model::raw(
    "SELECT * FROM users WHERE email = :email AND plan = :plan",
    json::Object{{"email", e}, {"plan", "premium"}});

// Typed hydration
std::vector<User> premium = Model::rawAs<User>(
    "SELECT * FROM users WHERE plan = :plan",
    json::Object{{"plan", "premium"}});

// Non-SELECT (auto-detect)
Model::raw("REFRESH MATERIALIZED VIEW leaderboard");

// В typed chain
auto rows = User::query<User>()
    ->raw("SELECT id, email FROM users WHERE plan = ?", { "premium" })
    ->get();

// Mongo envelope
json out = Model::rawJson(
    json::Object{{"collection", "users"}, {"filter", ...}});
```

**Забележка.** Instance-level `Model::raw()` chain (връщащ `Model*`)
**не се излага** — C++ не позволява overload между static и instance
метод със същата сигнатура. Ползвайте `TypedQuery<T>::raw()` за
chain композиция или директно `model.getBuilder()->raw(...)`.

### 10. SingleStore / MemSQL backend (2026-09-21)

SingleStore (fka MemSQL) е MySQL wire-protocol compatible. Vendor-ът
го третира като отделен backend (`DATABASE_TYPE=singlestore` в `.env`),
но reuse-ва `libmysqlcppconn` за transport — не се въвежда нов native
client.

**Компоненти.**

- `orm/connection/singlestore_connection.{h,cpp}` — thin wrapper върху
  `libmysqlcppconn`; тялото е идентично на `MysqlConnection`. Държан
  като отделен клас за симетрия и за бъдещи SingleStore-specific
  hooks (distributed timeout retry, `USE PARTITION` hints).
- `orm/grammar/singlestore_grammar.{h,cpp}` — `SinglestoreGrammar :
  public MySqlGrammar`. В първата ревизия няма override-и; наследява
  backticks, `?` placeholders и `JSON_ARRAYAGG` / `JSON_OBJECT` (7.5+)
  emitter-а. Класът съществува като anchor point за бъдещи DDL
  divergence-и.
- `DbFactory("singlestore")` route-ва към горните двата класа.
- Backend allow-list-ите в `migrate/*` (Migration, MigrationRunner,
  MigrationStore, `migrate_main` help) знаят за `"singlestore"`.
- Compile-time smoke check: `orm/singlestore_smoke.cpp` държи factory
  branch-а жив под LTO и static-asserts inheritance chain-а.

**`.env` пример.** Prefix-ът (`SING`, `MEMSQL`, …) е arbitrary — избира се
от consumer-а и се сочи чрез `MIGRATION_DB`. Важна е стойността
`singlestore` на `<PREFIX>_DATABASE_TYPE`.

```
MIGRATION_DB=SING

SING_DATABASE_TYPE=singlestore
SING_DATABASE_HOST=127.0.0.1
SING_DATABASE_PORT=3306
SING_DATABASE_NAME=mydb
SING_DATABASE_USER=root
SING_DATABASE_PASSWORD=...
```

**Известни ограничения** (vendor-ът не ги guard-ва — user отговорност):

- `UPDATE ... LIMIT` не се поддържа върху distributed таблици.
- Транзакциите имат restricted scope (виж SingleStore docs).
- DDL за `SHARD KEY` / `SORT KEY` / `ROWSTORE|COLUMNSTORE` се пише raw
  в миграциите; fluent schema DSL за тях няма (виж §6/§7 gap list).

**Native SingleStore C client** не се използва — MySQL protocol
покрива 100% от query pipeline-а (SELECT/INSERT/UPDATE/DELETE + JSON
aggregation). Ако в бъдеще се появи публичен native driver, може да
се добави като алтернативен connection зад същия backend key.

### 9. Hygiene

- `[[nodiscard]]` върху всички query terminals и getters.
- `std::string_view` overloads в `where`, `sanitizeOperator`,
  `sanitizeOrderBy`, `assertSafeIdentifier` — премахва излишни string
  копия.
- Operator allowlist е сега `constexpr std::array` — няма runtime
  heap allocation за static set.
- Include guards преименувани към `GARVAN_*`.
- `#include <pqxx/pqxx>` премахнат от `orm/omodel.h` (ORM-neutral header).
- **`GARVAN_SQL_DEBUG=1`** (env flag) — stderr trace на компилирания SQL за
  non-raw пътищата: `[Garvan::sql] <SQL>  (params=N)`. Огледален на
  `[Garvan::raw]`; cache-ва се еднократно, без per-query `getenv()` cost.

### 11. Query surface expansion (2026-09-26)

Query builder-ът получава набор от Laravel-parity операции: OR
WHERE, IN / NOT IN, ORDER BY, structured SELECT, COUNT(*) и aliased
JOINs. Всички са налични на `Builder`, `Model` и `TypedQuery<T>` с
идентична сигнатура.

**11.1 OR WHERE.** `WhereClause` получава `connector` поле
(`"AND"` default / `"OR"`); `compileWheres` рендва свързвателя между
съседни клаузи. `sanitizeConnector` — allow-list `{AND, OR}`.

```cpp
User::query<User>()
    ->where("plan", "=", "premium")
    ->orWhere("trial_expires_at", ">", now)
    ->get();
```

`orWhere` overloads-ите огледалят `where`: string / typed
(`integral`, `floating_point`, `bool`) / `nullptr_t`.

**11.2 WHERE IN / NOT IN.**

```cpp
User::query<User>()
    ->whereIn("id", {"1","2","3"})
    ->orWhereNotIn("status", {"banned","deleted"})
    ->get();
```

Всяка стойност — собствен placeholder. Празен list → `runtime_error`.

**11.3 Model IS NULL.** `Model::where(field, op, nullptr)` и
`Model::orWhere(...)` — `<col> IS [NOT] NULL` inline (no bind).

**11.4 ORDER BY.** `orderBy(col, dir="asc")` (asc|desc,
case-insensitive; column минава през `sanitizeOrderBy`), plus
`orderByRaw(expr)` за multi-column и backend extensions
(`NULLS LAST`). Overwrite semantics (не stack).

**11.5 Structured SELECT / selectRaw / withPrivate.** Нов
`SelectClause { kind, expr, alias }` с 3 варианта: `Column`
(wrapped identifier), `Aliased` (wrapped + AS), `Raw` (verbatim
SQL — caller-guarded).

```cpp
User::query<User>()
    ->select("users.id")
    ->selectAs("users.email", "user_email")
    ->selectRaw("COALESCE(NULLIF(t.name_bul,''),t.name_eng)", "display_name")
    ->get();
```

Precedence: `selects` (non-empty) > `withPrivate()` > `public_columns`.
`withPrivate()`: SELECT `<table>.*` — включва private колони
(password hashes и т.н.).

**11.6 COUNT(\*).**

```cpp
int64_t n = User::query<User>()->where("active","=",true)->count();
```

Exception-safe state restore на `countMode` / `limit`.
`parseCountEnvelope` handles list-of-object, bare object и
numeric-as-string envelope shapes.

**11.7 Aliased JOINs.**

```cpp
Games::query<Games>()
    ->leftJoinAs("users", "w", "w.id", "=", "games.white_id")
    ->leftJoinAs("users", "b", "b.id", "=", "games.black_id")
    ->selectAs("w.username", "white_name")
    ->get();
```

`JoinClause::alias` — рендва ` <TYPE> JOIN <table> AS <alias>`.
`wrapQualified` работи над alias references.

**11.8 `wrapModelColumn(id, modelTable)`.** Auto-qualify на bare
model-owned колони под JOIN — `"id"` → `"<modelTable>"."id"`. Fix за
`Duplicate column name` (MySQL/SingleStore) и ambiguous reference
(Postgres) при JOIN-нати таблици с колидиращи имена.

**11.9 `DbClient::lastInsertId()`.** Portable surrogate за
`RETURNING id` / `LAST_INSERT_ID()` / `sqlite3_last_insert_rowid`.

**11.10 Model default client.** `Model(std::string client = "")` —
по-рано `"PSQL"`. Празен string → `DbFactory` resolve-ва от `.env`.
Explicit `Model("PSQL")` продължава да работи.

**BC.** Всички добавки са additive. Единствена behavioural промяна:
Model default-ът (`""` вместо `"PSQL"`) — consumer-и с empty `.env`,
разчитащи на Postgres implicit, трябва да минат explicit.

## Queue & Events subsystem

Vendor-ът включва background pipeline за jobs и events, който starter-ското
приложение окабелява през service provider-и. Компоненти:

- `queue/Job.h` — базов клас за job. Override-ва се `handle()`, `jobName()`,
  `payload()`, `connection()`, плюс статичен `fromPayload(const JsonValue&)`.
- `queue/JobRegistry.{h,cpp}` — global registry `name → factory`.
  `bind("Name", &Class::fromPayload)`, `make("Name", payload)`, `names()`,
  `size()`.
- `queue/JobDispatcher.{h,cpp}` — рутира по `job->connection()` към
  съответния driver.
- `queue/drivers/SyncDriver` — inline execution в calling thread.
  Async / database driver-и се bind-ват от starter-a през
  `AppServiceProvider`.
- `events/Event.h`, `events/EventRegistry.{h,cpp}`,
  `events/EventDispatcher.{h,cpp}` — идентичен pattern за events.
  `Listener<T>` = typed listener base.

⚠️ **Известна inconsistency.** `GARVAN_REGISTER_JOB(NS::Class)` в
`queue/JobRegistry.h:58` регистрира ключа като stringified token
(`"NS::Class"`), а не като `T{}.jobName()`. Ако искаш `kalpasan
job:dispatch ShortName` да работи, ползвай директно:

```cpp
Garvan::JobRegistry::bind("ShortName", &NS::Class::fromPayload);
```

в `JobServiceProvider`-a на consumer-a. Планирано: макросът да мине
към `T{}.jobName()` в бъдеща версия (изисква default-constructible T).

## Kalpasan runtime verbs

Kalpasan съдържа CLI глаголи, които говорят с работещото user-ско
приложение по HTTP (libcurl). Регистрирани в `Kalpasan.cpp:53-56`:

| Verb                              | HTTP endpoint                        |
| --------------------------------- | ------------------------------------ |
| `job:list`                        | `GET  /api/admin/jobs`               |
| `job:dispatch <name> [--field]`   | `POST /api/admin/jobs/dispatch`      |
| `event:list`                      | `GET  /api/admin/events`             |
| `event:fire <name> [--field]`     | `POST /api/admin/events/fire`        |

Всички четат `KALPASAN_ADMIN_URL` (default `http://127.0.0.1:9090`) и
`KALPASAN_ADMIN_TOKEN` от `.env`; изпращат `Authorization: Bearer <token>`.
Consumer-ското приложение трябва да е live и да е монтирало `AdminRoute`
(loopback-only + bearer guard).

## Migrations и Kalpasan CLI

Виж `garvan-migrate help` и `kalpasan help`.

## Синхронизация към consumer

`make.sh` копира `libgarvan.a`, `garvan-migrate`, `kalpasan`, `crow.h`
и всички public headers в `../garvancpp-starter/vendors/Garvan/`
автоматично след успешен build.
