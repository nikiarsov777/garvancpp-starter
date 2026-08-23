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
User::query<User>()->where("active","=", false)
                   ->update({{"deleted_at", now}});
FcmToken::query<FcmToken>()->where("expires_at","<", now)
                           ->remove();

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

### 7. Hygiene

- `[[nodiscard]]` върху всички query terminals и getters.
- `std::string_view` overloads в `where`, `sanitizeOperator`,
  `sanitizeOrderBy`, `assertSafeIdentifier` — премахва излишни string
  копия.
- Operator allowlist е сега `constexpr std::array` — няма runtime
  heap allocation за static set.
- Include guards преименувани към `GARVAN_*`.
- `#include <pqxx/pqxx>` премахнат от `orm/omodel.h` (ORM-neutral header).

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
