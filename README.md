# Garvan C++

A fast, batteries-included C++ web framework starter — Crow under the hood, with an
ORM/query builder, migrations, a model/controller/service tier, multi-database
drivers and the `kalpasan` CLI.

`C++20`  ·  `CMake >= 3.20`  ·  `License GPL-3.0`

---

## English

### About

This repository is a working starter application for the **Garvan** C++ web
framework. The framework itself ships pre-built in `vendors/Garvan/`
(`libgarvan.a`, the bundled Crow headers, the `garvan-migrate` binary and the
`kalpasan` CLI). The rest of the tree is a small example app you can edit, build
and deploy.

### Features

- HTTP and WebSocket server built on Crow's compile-time routing core.
- Fluent ORM / query builder with per-backend grammar (Postgres, MySQL, SQLite,
  MongoDB, MonetDB).
- Migrations and seeders driven by the standalone `garvan-migrate` tool.
- Mustache templating with a multi-language built-in documentation site
  (en, bg, es, pt, ru, tr).
- Scaffolding via the `kalpasan` CLI: models, controllers, services, migrations.
- MVC-style layout: `app/models`, `app/controllers`, `app/services`, `routes/`.
- Event bus + job queue with pluggable drivers (`SyncDriver` today; async / database
  drivers pluggable via service providers).
- Real SMTP mail sending (HTML, TLS / STARTTLS) via libcurl in `AppJobs::SendTestMail`.
- Typed ORM pipeline (`Model::query<T>()`, `TypedQuery<T>`) with automatic
  rehydration, instance-level `save()`/`remove()`, and safe bulk `update()`/
  `remove()` (see "Typed ORM pipeline" below).

### Project structure

```
.
├── app/
│   ├── controllers/        # web controllers
│   │   └── api/            # JSON API controllers (psql, mysql, mongo, monet)
│   ├── models/             # User, Role, Team, Post, Comment, ...
│   ├── services/           # business logic per backend
│   ├── events/             # AppEvents::UserRegistered ...
│   ├── jobs/               # AppJobs::SendTestMail (real SMTP via libcurl)
│   ├── listeners/          # LogRegistrationListener, SendWelcomeEmailListener
│   └── providers/          # AppServiceProvider, JobServiceProvider, EventServiceProvider
├── routes/
│   ├── ApiRoutes.cpp       # /api/* endpoints
│   ├── WebRoutes.cpp       # / and language switch
│   ├── DocsRouter.cpp      # multi-language docs router
│   ├── JobRoute.cpp        # /api/jobs/* (direct job dispatch + status)
│   ├── EventRoute.cpp      # /api/events/* (fire events + status)
│   └── AdminRoute.cpp      # /api/admin/{jobs,events}* (bearer-guarded; used by kalpasan)
├── db/
│   ├── migrations/         # *.<backend>.sql files
│   └── seeders/            # *.sql seeders run by kalpasan
├── public/                 # mustache templates + multi-language docs pages
├── static/                 # css, js, images
├── vendors/Garvan/         # libgarvan.a, crow headers, kalpasan, garvan-migrate
├── main.cpp                # entry point — boots Crow on port 9090
├── CMakeLists.txt          # primary build
└── .env                    # runtime configuration
```

The HTTP server is started in `main.cpp:16` and listens on **port 9090** by default.

### Requirements

- GCC 11+ or Clang 12+ (C++20) for building the starter app.
- **Rebuilding the vendor from source** (`libgarvan.a`) requires **GCC 14+
  or Clang 18+** (C++23: `deducing this`, `std::expected`) and CMake 3.16+.
- CMake 3.20 or later.
- Asio 1.28+ development headers.
- Database client libraries: `libpqxx`, `libmysqlcppconn`, `libsqlite3`,
  `libmongoc` / `libbson` (mongocxx), `libmonetdb-mapi`.
- `libcurl` (SMTP transport for `AppJobs::SendTestMail`; also used by
  `kalpasan` for talking to the admin API).
- Optional: OpenSSL (HTTPS) and zlib (compression).

Debian / Ubuntu one-liner:

```bash
sudo apt update
sudo apt install -y \
    build-essential cmake \
    libasio-dev libssl-dev zlib1g-dev \
    libcurl4-openssl-dev \
    libpqxx-dev libmysqlcppconn-dev libsqlite3-dev \
    libmongoc-dev libbson-dev
```

CMake picks libcurl up automatically via `find_package(CURL REQUIRED)` and
links `CURL::libcurl` into `app_bin`.

### Getting started

```bash
git clone https://github.com/nikiarsov777/garvan-starter.git my-app
cd my-app
cp .env.example .env
# edit .env — credentials, host, port, database name

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

./bin/app.bin
```

Then open <http://localhost:9090>.

### Alternative build (plain g++)

A ready-to-use `make.sh` script lives at the repo root. Run it with:

```bash
./make.sh
```

It produces the same `bin/app.bin` without involving CMake. Full contents:

```bash
#!/bin/bash
# Build script for Garvan C++ app -> bin/app.bin
# Mirrors CMakeLists.txt so you can build without cmake.

set -e

# Clean & recreate output dir
rm -rf bin
mkdir -p bin

g++ -std=c++20 -O2 -pthread \
    main.cpp \
    routes/*.cpp \
    app/controllers/*.cpp \
    app/controllers/api/*.cpp \
    app/models/*.cpp \
    app/services/*.cpp \
    -I. \
    -Iapp/models \
    -Iapp/controllers \
    -Ivendors/Garvan \
    -Ivendors/Garvan/include \
    -Ivendors/Garvan/db \
    -Ivendors/Garvan/model \
    -Ivendors/Garvan/orm \
    -Ivendors/Garvan/orm/connection \
    -Ivendors/Garvan/orm/grammar \
    -Ivendors/Garvan/service \
    -Ivendors/Garvan/tools \
    -I/usr/include/monetdb \
    $(pkg-config --cflags libmongocxx libbsoncxx) \
    vendors/Garvan/libgarvan.a \
    -L/usr/lib/x86_64-linux-gnu \
    -lpqxx -lpq \
    -lmysqlcppconn \
    -lmapi \
    -lsqlite3 \
    $(pkg-config --libs libmongocxx libbsoncxx) \
    -o bin/app.bin

echo "Build OK -> bin/app.bin"
```

### Configuration (`.env`)

The shipped `.env` keeps a separate block per database so you can switch with one
variable. `MIGRATION_DB` decides which block `garvan-migrate` will read.

```dotenv
MIGRATION_DB=PSQL

# PostgreSQL
PSQL_DATABASE_TYPE=postgres
PSQL_DATABASE_HOST=127.0.0.1
PSQL_DATABASE_PORT=5432
PSQL_DATABASE_NAME=
PSQL_DATABASE_USER=
PSQL_DATABASE_PASSWORD=

# MySQL
MYSQL_DATABASE_TYPE=mysql
MYSQL_DATABASE_HOST=localhost
MYSQL_DATABASE_PORT=3306
MYSQL_DATABASE_NAME=
MYSQL_DATABASE_USER=
MYSQL_DATABASE_PASSWORD=

# MongoDB
MONGODB_DATABASE_TYPE=mongodb
MONGODB_DATABASE_HOST=127.0.0.1
MONGODB_DATABASE_PORT=27017
MONGODB_DATABASE_NAME=
MONGODB_DATABASE_USER=
MONGODB_DATABASE_PASSWORD=

# MonetDB
MONETDB_DATABASE_TYPE=monetdb
MONETDB_DATABASE_HOST=127.0.0.1
MONETDB_DATABASE_PORT=50000
MONETDB_DATABASE_NAME=garvan
MONETDB_DATABASE_USER=
MONETDB_DATABASE_PASSWORD=
MONETDB_DATABASE_SCHEMA=sys
```

`APP_KEY` is mandatory. The server refuses to start if it is missing, empty,
or whitespace-only. Generate one with `openssl rand -base64 32` and set
`APP_KEY=base64:<value>` in `.env` (or export it in the process environment).

Never commit your real `.env` — only `.env.example` belongs in version control.

### Migrations and seeders

```bash
./vendors/Garvan/garvan-migrate up        # apply pending migrations
./vendors/Garvan/garvan-migrate down      # revert the last applied migration
./vendors/Garvan/garvan-migrate status    # show pending / applied
```

Migration filenames follow `<UTC_YYYYMMDDhhmmss>_<description>.<backend>.sql`.
The backend suffix may be `postgres`, `mysql`, `mongodb` or `monetdb`; a plain
`.sql` file is treated as backend-agnostic. Each file declares an
`-- migrate:up` and an `-- migrate:down` section. MongoDB migrations contain
JSON documents that are forwarded to `db.runCommand()`.

Seeders live in `db/seeders/` and are executed by `kalpasan`:

```bash
./kalpasan db:seed --class=UserSeeder
```

### Troubleshooting the ORM

If the bundled ORM in `vendors/Garvan/libgarvan.a` gives you trouble — link
errors, missing symbols, an incompatible system `libpqxx` / `mysqlcppconn` /
`mongocxx`, or unexpected behaviour from the query builder — you can rebuild
it from source and drop the artifact back into `vendors/Garvan/`:

```bash
git clone https://github.com/nikiarsov777/garvancpp-vendor-pub.git
cd garvancpp-vendor-pub
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

Copy the freshly built `libgarvan.a` (and any updated headers under `include/`)
over the files in your project's `vendors/Garvan/` directory, then rebuild the
app with `cmake --build build -j` or `./make.sh`.

The vendor tree ships its own `make.sh` — a thin wrapper around CMake
that performs an incremental build and then **auto-syncs** the
resulting `libgarvan.a`, `garvan-migrate`, `kalpasan`, `crow.h` and
public headers into `../garvancpp-starter/vendors/Garvan/`:

```bash
./make.sh              # incremental build + sync
./make.sh clean        # full rebuild
JOBS=8 ./make.sh       # override parallel jobs (default: 4)
```

Never pass a bare `-j` to `cmake --build` in the vendor tree — with the
Makefile generator that expands to `make -j` (unbounded jobs → swap
thrashing). The vendor script always passes a concrete `-j "$JOBS"`.

### Typed ORM pipeline (`TypedQuery<T>`)

The classic Garvan chain (`Model::where<T>(...)->first()`) returns
`json` and does not rehydrate into a typed model. The typed pipeline
(added 2026-08-21) closes that gap without breaking any existing
call site:

```cpp
// (1) Chain by any column, mutate, save (UPDATE)
User u = User::query<User>()
           ->where("email", email)
           ->where("active","=", true)
           ->firstOrFail();
u.set("last_login", now);
u.save();                          // UPDATE users SET ... WHERE id=<hydrated>

// (2) Load by id
User u = User::findAs<User>(id);                     // throws if missing
auto  opt = User::tryFindAs<User>(id);               // std::optional<User>

// (3) Instance-level DELETE
User u = User::findAs<User>(id);
u.remove();                        // DELETE FROM users WHERE id=<hydrated>

// (4) Bulk UPDATE / DELETE — empty WHERE throws

// Status transition (state machine on a cohort)
Invoice::query<Invoice>()->where("status","=","pending")
                        ->update({{"status","sent"},
                                  {"sent_at", now}});

// Soft-delete on a coherent cohort
User::query<User>()->where("banned","=", true)
                   ->update({{"deleted_at", now}});          // soft-delete

// Hard-delete stale rows
FcmToken::query<FcmToken>()->where("expires_at","<", now)
                           ->remove();                        // hard delete

// (5) WHERE ... IS NULL on the typed surface
Settings s = Settings::query<Settings>()
               ->where("user_id","IS", nullptr)
               ->firstOrFail();

// (6) List rehydration
std::vector<User> active = User::query<User>()
                             ->where("active","=", true)
                             ->get();
```

**Terminal cheat sheet:**

| Method                       | Returns                    | On no-match |
| ---------------------------- | -------------------------- | ----------- |
| `first()`                    | `std::optional<T>`         | `nullopt`   |
| `firstOrFail()`              | `T`                        | throws      |
| `find(int id)`               | `std::optional<T>`         | `nullopt`   |
| `findOrFail(int id)`         | `T`                        | throws      |
| `get()`                      | `std::vector<T>`           | empty       |
| `update(json fields)`        | void                       | throws if no WHERE |
| `remove()`                   | void                       | throws if no WHERE |
| `Model::findAs<T>(id)`       | `T`                        | throws      |
| `Model::tryFindAs<T>(id)`    | `std::optional<T>`         | `nullopt`   |
| `Model::query<T>()`          | `std::unique_ptr<TypedQuery<T>>` | —     |

Setters remain manual on subclasses (`this->set("email", v)` inside a
scaffolded `UserMonet::setEmail(...)` helper); no macro-generated
accessors are introduced in this revision.

**Guards.** Bulk `update()` / `remove()` refuse to run without at
least one `where(...)` clause (protection against accidental full-
table writes). `Model::remove()` on an unhydrated instance (empty id)
also throws.

**BC.** The classical `Model::where<T>(...)` chain and the untyped
`json first() / find(int) / get()` terminals stay untouched.

**Behind the scenes.** Connection layers ship results as
`JsonValue::RawJson` (a JSON string). `TypedQuery` parses that back
into a walkable Object/Array tree via the new
`JsonValue::parse(std::string_view)` (RFC 8259, dependency-free) and
calls `Model::hydrate(row)` — which populates `attributes` and sets
`id` so the next `save()` naturally hits the UPDATE branch.

Closed gaps from `vendors/Garvan/GARVAN.md`: **Gap 1** (Model IS NULL
— typed surface), **Gap 2** (DELETE — instance + bulk), **Gap 6**
(UPDATE with non-PK WHERE), **Gap 7** (rehydration into model
instance). Still open: Gap 3 (aggregates), Gap 4 (JOIN emission),
Gap 5 (last-insert-id), Gap 8 (MonetDB PS bind pipeline).

### C++23 ORM surface

Alongside the typed pipeline the vendor exposes a family of C++23-only
APIs (all shipped in parallel to the classical ones — full BC is
preserved). To rebuild the vendor from source you need **GCC 14+ or
Clang 18+** (for `deducing this` and `std::expected`).

**1. `ModelType` concept for static factories.**

```cpp
template <ModelType T>
static T* where(std::string field, std::string value);
// ModelType = std::derived_from<T, Garvan::Model>
//          && std::is_default_constructible_v<T>
```

Errors on a wrong `T` are short and readable instead of pages of
template noise.

**2. No more `delete this` — scratchpad ownership.**

Terminal methods (`get`, `find`, `first`, …) no longer `delete this`.
Stack instances are safe. Models returned from static `where<T>()`
live in a `thread_local` scratchpad, cleared on the next `where<T>()`
on the same thread or via an explicit call:

```cpp
Garvan::Model::flushScratch();
```

**3. Typed WHERE overloads.**

```cpp
builder->where("id", "=", 42);                // int  -> "42"
builder->where("active", "=", true);          // bool -> "true"
builder->where("deleted_at", "IS", nullptr);  //      -> "NULL"
```

Concept-guarded overloads for `integral` / `floating_point` / `bool` /
`nullptr_t`. The existing string API stays intact.

**4. Deducing-`this` fluent value chain.**

```cpp
Builder b(...);
b.whereRef("id", "1").whereRef("age", ">", "18").get();
```

`whereRef` returns `Self&` instead of `Self*`, enabling value chains
on stack builders. The old `Builder* where(...)` is kept for BC.

**5. `std::expected<json, DbError>` instead of exceptions.**

```cpp
auto result = user->tryFirst();
if (!result) {
    log(result.error().message);
    return;
}
json data = *result;
```

`DbError::Code = { Connection, Syntax, Constraint, NotFound, Unknown }`.
Classification is currently heuristic (matched on `what()`); precise
SQLSTATE-driven classification lands in a future connection-layer
extension. Throwing methods (`get`, `first`, `find*`) are kept for BC.

**6. Hygiene.**

- `[[nodiscard]]` on all query terminals and getters.
- `std::string_view` overloads in `where`, `sanitizeOperator`,
  `sanitizeOrderBy`, `assertSafeIdentifier`.
- Operator allowlist is a `constexpr std::array` — no runtime heap
  allocation for the static set.
- Include guards renamed to `GARVAN_*`.
- `#include <pqxx/pqxx>` removed from `orm/omodel.h` (ORM-neutral
  header).

See `vendors/Garvan/README.md` for the full vendor-side change log.

### Kalpasan CLI

`kalpasan` is symlinked at the repo root and points at
`vendors/Garvan/kalpasan`. All commands read the active `.env` file.

| Command                                | What it does                                       |
| -------------------------------------- | -------------------------------------------------- |
| `./kalpasan make:model <Name>`         | Scaffold `app/models/<Name>.{h,cpp}`               |
| `./kalpasan make:controller <Name>`    | Scaffold `app/controllers/<Name>Controller.{h,cpp}`|
| `./kalpasan make:service <Name>`       | Scaffold `app/services/<Name>Service.{h,cpp}`      |
| `./kalpasan make:migration <name>`     | Create a new file in `db/migrations/`              |
| `./kalpasan db:migrate`                | Delegates to `garvan-migrate up`                   |
| `./kalpasan db:rollback`               | Delegates to `garvan-migrate down`                 |
| `./kalpasan db:seed`                   | Run files in `db/seeders/`                         |
| `./kalpasan serve --watch`             | Rebuild and restart on `.cpp` / `.h` change        |
| `./kalpasan job:list`                  | `GET /api/admin/jobs` — list registered job classes|
| `./kalpasan job:dispatch <Name> [--field k=v]` | `POST /api/admin/jobs/dispatch` — dispatch a job with JSON payload |
| `./kalpasan event:list`                | `GET /api/admin/events` — list registered events   |
| `./kalpasan event:fire <Name> [--field k=v]`   | `POST /api/admin/events/fire` — fire an event with payload         |
| `./kalpasan route:list`                | Print HTTP routes                                  |
| `./kalpasan config:show` / `config:get` / `config:set` | Inspect / edit `.env` values          |
| `./kalpasan config:key:generate`       | Generate a new `APP_KEY` (base64 32-byte)          |

Runtime verbs (`job:*`, `event:*`) are HTTP clients that talk to the
running app via loopback. They read `KALPASAN_ADMIN_URL` (default
`http://127.0.0.1:9090`) and `KALPASAN_ADMIN_TOKEN` from `.env`. The
`AdminRoute` handler in the app requires all three: a non-empty token,
a loopback source IP (`127.0.0.1` or `::1`), and a matching `Bearer`
header — otherwise it returns 401/403/503 with a JSON error body.

### Jobs & Events

The app boots three service providers in `main.cpp` (order matters —
`AppServiceProvider` first, then jobs, then events):

```
AppKernel::bootAll()
  ├── AppServiceProvider          (binds queue drivers)
  ├── JobServiceProvider          (registers job classes)
  └── EventServiceProvider        (registers events + listeners)
```

End-to-end pipeline exercised by the sample event:

```
GET /api/events/user-registered?email=...&name=...&id=...
  → EventDispatcher::fire(UserRegistered)
      → LogRegistrationListener::handle()      (sync log line)
      → SendWelcomeEmailListener::handle()
           → JobDispatcher::dispatch(SendTestMail)
                → SyncDriver (inline, this thread)
                     → SendTestMail::handle()  (libcurl SMTP send)
```

**Adding a new job**

1. Create `app/jobs/MyJob.{h,cpp}` deriving from `Garvan::Job`, override
   `handle()`, `jobName()`, `payload()` and provide a static
   `fromPayload(const Garvan::JsonValue&)` factory.
2. Register it in `app/providers/JobServiceProvider.cpp` with:
   ```cpp
   Garvan::JobRegistry::bind("MyJob", &AppJobs::MyJob::fromPayload);
   ```
   ⚠️ Do **not** use `GARVAN_REGISTER_JOB(AppJobs::MyJob)` for user jobs.
   The macro stringifies the whole token and registers the key as
   `"AppJobs::MyJob"`, which then does not match `jobName()` (or the
   short name expected by `kalpasan job:dispatch`). See
   `app/providers/JobServiceProvider.cpp:18` for the pattern in use.
3. Rebuild (`./make.sh`) and restart `./bin/app.bin`.

**Adding a new event + listener**

1. Create the event class in `app/events/` (derive from `Garvan::Event`).
2. Create the listener in `app/listeners/` (derive from
   `Garvan::Listener<YourEvent>`) and override `handle()`.
3. Register both in `app/providers/EventServiceProvider.cpp`
   (`EventDispatcher::listen<...>()` + `GARVAN_REGISTER_EVENT(...)`).

### Mail configuration

`AppJobs::SendTestMail` sends a real HTML email over SMTP using libcurl.
It reads the `MAIL_*` block from `.env`:

```dotenv
MAIL_DRIVER=smtp
MAIL_HOST=smtp.example.com
MAIL_PORT=465                    # 465 -> implicit TLS (smtps://)
                                 # 587 -> STARTTLS   (smtp:// + CURLUSESSL_ALL)
MAIL_USERNAME="user@example.com"
MAIL_PASSWORD="app-password"
MAIL_ENCRYPTION=tls              # ssl / smtps -> force implicit TLS
MAIL_FROM_ADDRESS="user@example.com"
MAIL_FROM_NAME="Your App"
MAIL_AUTHENTICATION=plain        # informational; libcurl auto-negotiates
```

The RFC 822 message is `Content-Type: text/html; charset=UTF-8` with
8-bit CTE, so the `body` field may contain arbitrary HTML.

**Debugging SMTP dialog**

If a send silently fails or ends up in spam, temporarily flip verbose
mode in `app/jobs/SendTestMail.cpp`:

```cpp
// Set to 1L при debug на TLS handshake / SMTP dialog.
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);   // ← change 0L to 1L
```

Rebuild, restart, retry, then read the full TLS handshake + SMTP
dialog printed to the server's stdout. **Do not commit this change** —
production logs should stay clean.

### Routes

JSON API (registered in `routes/ApiRoutes.cpp`):

| Method | Path                                  | Notes                                |
| ------ | ------------------------------------- | ------------------------------------ |
| GET    | `/api/psql/users`                     | List users from PostgreSQL           |
| POST   | `/api/psql/users`                     | Create user in PostgreSQL            |
| GET    | `/api/psql/users/<int>`               | Fetch one user by id                 |
| GET    | `/api/psql/users/<int>/posts`         | User's posts (hasMany)               |
| GET    | `/api/psql/users/<int>/roles`         | User's roles (belongsToMany)         |
| GET    | `/api/psql/teams`                     | List teams                           |
| GET    | `/api/psql/roles`                     | List roles                           |
| GET    | `/api/psql/posts`                     | List posts                           |
| GET    | `/api/mysql/users`                    | List users from MySQL                |
| POST   | `/api/mysql/users`                    | Create user in MySQL                 |
| GET    | `/api/monetdb/users`                  | List users from MonetDB              |
| POST   | `/api/monet/users`                    | Create user in MonetDB               |
| GET    | `/api/mongo/users`                    | List users from MongoDB              |
| POST   | `/api/mongo/users`                    | Create user in MongoDB               |

Jobs, events and admin (all served by `main.cpp`):

| Method | Path                              | Notes                                         |
| ------ | --------------------------------- | --------------------------------------------- |
| GET    | `/api/jobs/send-mail`             | Direct `SendTestMail` dispatch (bypass event bus). Query: `to`, `subject`, `body` |
| GET    | `/api/jobs/status`                | Bound drivers + registry size                 |
| GET    | `/api/events/user-registered`     | Fire `UserRegistered`. Query: `email`, `name`, `id` |
| GET    | `/api/events/status`              | Listener counts per known event               |
| GET    | `/api/admin/jobs`                 | List registered jobs (bearer + loopback)      |
| POST   | `/api/admin/jobs/dispatch`        | Dispatch by name + JSON payload (bearer + loopback) |
| GET    | `/api/admin/events`               | List registered events (bearer + loopback)    |
| POST   | `/api/admin/events/fire`          | Fire by name + JSON payload (bearer + loopback) |

Web (`routes/WebRoutes.cpp`): `GET /` serves the docs home, `GET /lang/<code>`
sets the language cookie and redirects back, and the catch-all route resolves
any other path against the built-in docs index.

### Quick API examples

```bash
# List users
curl http://localhost:9090/api/psql/users

# Create a user
curl -X POST http://localhost:9090/api/psql/users \
     -H "Content-Type: application/json" \
     -d '{"name":"Ada","email":"ada@example.com","password":"secret"}'

# Fire an event → runs the full listener chain → sends real mail
curl "http://localhost:9090/api/events/user-registered?email=you@x.com&name=You&id=1"

# Direct job dispatch (bypass event bus) with HTML body
curl "http://localhost:9090/api/jobs/send-mail?to=you@x.com&subject=Hi&body=<h1>Hello</h1>"

# Debug endpoints
curl http://localhost:9090/api/jobs/status
curl http://localhost:9090/api/events/status

# Admin API (same channel as `kalpasan job:dispatch`)
curl -X POST http://localhost:9090/api/admin/jobs/dispatch \
     -H 'Content-Type: application/json' \
     -H "Authorization: Bearer $KALPASAN_ADMIN_TOKEN" \
     -d '{"job":"SendTestMail","payload":{"to":"you@x.com","subject":"Hi","body":"<p>HTML</p>"}}'
```

### Built-in documentation site

When the app is running, the framework's own docs are served from the root URL.
Useful entry points:

- `/getting_started/setup_linux` — install and build.
- `/getting_started/your_first_application` — minimal Crow handler walk-through.
- `/guides/app`, `/guides/routes`, `/guides/middleware`, `/guides/websockets`.
- `/garvan/orm`, `/garvan/models`, `/garvan/migrations`, `/garvan/kalpasan`,
  `/garvan/databases`, `/garvan/env`.

All pages are available in English, Bulgarian, Spanish, Portuguese, Russian and
Turkish — switch with `GET /lang/<code>`.

### Internationalisation (i18n)

The docs site (and any consumer page) is served through a
dictionary-backed i18n pipeline. Every page is **one canonical mustache
template** with `{{t_...}}` placeholders; the actual text lives in
per-language JSON dictionaries loaded at boot.

```
public/
├── langs/                     # one flat JSON dict per language
│   ├── en.json                # default + ultimate fallback
│   ├── bg.json
│   └── {ru,es,tr,pt}.json
└── pages/                     # 42 canonical templates (language-agnostic)
    ├── home.html, license.html, privacy.html, reference.html
    ├── garvan/            (13 pages)
    ├── getting_started/   (6 pages)
    └── guides/            (20 pages)
```

**Component map:**

| Component                       | File                          | Role                                                                       |
| ------------------------------- | ----------------------------- | -------------------------------------------------------------------------- |
| `AppServices::I18n`             | `app/services/I18n.{h,cpp}`   | Lazy-loads dicts, resolves keys with EN fallback, populates mustache ctx.  |
| `Routes::DocsRouter`            | `routes/DocsRouter.cpp`       | `PageMeta` index, route wiring, delegates translation to `I18n`.           |
| Crow mustache                   | (framework)                   | Renders `{{t_*}}` tokens against the injected context.                     |
| `tools/extract_docs_i18n.py`    | `tools/`                      | One-shot migration helper (DocsRouter dict → JSON).                        |
| `tools/extract_pages_i18n.py`   | `tools/`                      | One-shot migration helper (per-lang HTML → JSON + deploy canonicals).      |

**Fallback chain:** `current lang → EN → literal key name`.

**Language switching:** `GET /lang/<code>` sets a 1-year `lang` cookie;
`AppServices::I18n::langCookieHeader(lang)` builds the header value.

Full walkthrough with examples: see [`/garvan/i18n`](/garvan/i18n)
in the running docs site.

### License

GNU General Public License v3.0 — see [`LICENSE`](LICENSE).

---

## Български

### За проекта

Това хранилище е работещо стартово приложение за C++ уеб рамката **Garvan**.
Самата рамка е предварително компилирана в `vendors/Garvan/` (`libgarvan.a`,
хедърите на Crow, инструментът `garvan-migrate` и CLI-то `kalpasan`). Останалата
част от дървото е малък примерен проект, който можете да редактирате, билдвате и
деплойвате.

### Възможности

- HTTP и WebSocket сървър върху compile-time routing-а на Crow.
- ORM / query builder с граматика за всеки backend (Postgres, MySQL, SQLite,
  MongoDB, MonetDB).
- Миграции и сийдъри през самостоятелния `garvan-migrate`.
- Mustache шаблони и вградена многоезична документация (en, bg, es, pt, ru, tr).
- Скафолдинг през `kalpasan`: модели, контролери, услуги, миграции.
- MVC структура: `app/models`, `app/controllers`, `app/services`, `routes/`.
- Event bus + job queue със сменяеми driver-и (`SyncDriver` в момента; async /
  database driver-и се bind-ват през service provider-и).
- Реален SMTP mail send (HTML, TLS / STARTTLS) през libcurl в
  `AppJobs::SendTestMail`.
- Типизиран ORM pipeline (`Model::query<T>()`, `TypedQuery<T>`) с
  автоматична рехидратация, instance-ниво `save()`/`remove()` и
  безопасни bulk `update()`/`remove()` (виж "Типизиран ORM pipeline"
  по-долу).

### Структура на проекта

Същата структура като в английската секция по-горе. HTTP сървърът се стартира
в `main.cpp:16` на **порт 9090** по подразбиране.

### Изисквания

- GCC 11+ или Clang 12+ с поддръжка на C++20 за билд на starter-a.
- **Rebuild на vendor-а от source** (`libgarvan.a`) изисква **GCC 14+
  или Clang 18+** (C++23: `deducing this`, `std::expected`) и CMake 3.16+.
- CMake 3.20 или по-нов.
- Asio 1.28+ development хедъри.
- Драйвери: `libpqxx`, `libmysqlcppconn`, `libsqlite3`, `libmongoc` /
  `libbson` (mongocxx), `libmonetdb-mapi`.
- `libcurl` (SMTP транспорт за `AppJobs::SendTestMail`; ползва се и от
  `kalpasan` за admin API-то).
- По избор: OpenSSL за HTTPS и zlib за компресия.

За Debian / Ubuntu използвайте същата `apt install` команда от английската
секция (задължително и `libcurl4-openssl-dev`). CMake прихваща libcurl
автоматично през `find_package(CURL REQUIRED)` и линква `CURL::libcurl`.

### Бърз старт

```bash
git clone https://github.com/nikiarsov777/garvan-starter.git my-app
cd my-app
cp .env.example .env
# редактирайте .env — потребител, парола, хост, порт, база

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

./bin/app.bin
```

После отворете <http://localhost:9090>.

### Алтернативен билд (plain g++)

В основната директория има готов скрипт `make.sh`. Стартирайте го с:

```bash
./make.sh
```

Произвежда същия `bin/app.bin` без CMake. Пълно съдържание:

```bash
#!/bin/bash
# Build script for Garvan C++ app -> bin/app.bin
# Mirrors CMakeLists.txt so you can build without cmake.

set -e

# Clean & recreate output dir
rm -rf bin
mkdir -p bin

g++ -std=c++20 -O2 -pthread \
    main.cpp \
    routes/*.cpp \
    app/controllers/*.cpp \
    app/controllers/api/*.cpp \
    app/models/*.cpp \
    app/services/*.cpp \
    -I. \
    -Iapp/models \
    -Iapp/controllers \
    -Ivendors/Garvan \
    -Ivendors/Garvan/include \
    -Ivendors/Garvan/db \
    -Ivendors/Garvan/model \
    -Ivendors/Garvan/orm \
    -Ivendors/Garvan/orm/connection \
    -Ivendors/Garvan/orm/grammar \
    -Ivendors/Garvan/service \
    -Ivendors/Garvan/tools \
    -I/usr/include/monetdb \
    $(pkg-config --cflags libmongocxx libbsoncxx) \
    vendors/Garvan/libgarvan.a \
    -L/usr/lib/x86_64-linux-gnu \
    -lpqxx -lpq \
    -lmysqlcppconn \
    -lmapi \
    -lsqlite3 \
    $(pkg-config --libs libmongocxx libbsoncxx) \
    -o bin/app.bin

echo "Build OK -> bin/app.bin"
```

### Конфигурация (`.env`)

`.env` файлът съдържа отделен блок за всяка база и една променлива
`MIGRATION_DB`, която казва на `garvan-migrate` кой блок да чете. Вижте
английската секция за пълния пример.

`APP_KEY` е задължителен. Сървърът отказва да стартира, ако стойността
липсва, е празна или съдържа само интервали. Генерирайте такава с
`openssl rand -base64 32` и задайте `APP_KEY=base64:<стойност>` в `.env`
(или я експортирайте в средата на процеса).

> Никога не комитвайте реалния `.env` — само `.env.example` принадлежи на
> хранилището.

### Миграции и сийдъри

```bash
./vendors/Garvan/garvan-migrate up        # прилага новите миграции
./vendors/Garvan/garvan-migrate down      # връща последната миграция
./vendors/Garvan/garvan-migrate status    # показва какво е приложено
```

Имена на файловете: `<UTC_YYYYMMDDhhmmss>_<описание>.<backend>.sql`. Backend
суфиксът може да е `postgres`, `mysql`, `mongodb` или `monetdb`. Всеки файл има
две секции — `-- migrate:up` и `-- migrate:down`. Миграциите за MongoDB
съдържат JSON команди, които се пращат към `db.runCommand()`.

Сийдърите от `db/seeders/` се пускат през `kalpasan`:

```bash
./kalpasan db:seed --class=UserSeeder
```

### Отстраняване на проблеми с ORM

Ако вграденият ORM в `vendors/Garvan/libgarvan.a` създава проблеми — грешки
при линкване, липсващи символи, несъвместими системни `libpqxx` /
`mysqlcppconn` / `mongocxx`, или странно поведение на query builder-а —
можете да го компилирате наново от изходния код и да замените архива в
`vendors/Garvan/`:

```bash
git clone https://github.com/nikiarsov777/garvancpp-vendor-pub.git
cd garvancpp-vendor-pub
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

Копирайте новопостроения `libgarvan.a` (и евентуално обновените хедъри от
`include/`) върху файловете в `vendors/Garvan/` на проекта и пребилдвайте
приложението с `cmake --build build -j` или `./make.sh`.

Vendor дървото има свой `make.sh` — thin wrapper над CMake, който прави
инкрементален билд и после **автоматично синкроне** резултатните
`libgarvan.a`, `garvan-migrate`, `kalpasan`, `crow.h` и public хедъри
в `../garvancpp-starter/vendors/Garvan/`:

```bash
./make.sh              # incremental build + sync
./make.sh clean        # пълен rebuild
JOBS=8 ./make.sh       # override parallel jobs (default: 4)
```

Никога не подавай bare `-j` на `cmake --build` във vendor дървото — при
Makefile generator това става `make -j` (неограничен брой jobs → swap
thrashing). Скриптът винаги минава конкретен `-j "$JOBS"`.

### Типизиран ORM pipeline (`TypedQuery<T>`)

Класическият Garvan chain (`Model::where<T>(...)->first()`) връща
`json` и не рехидратира в типизиран модел. Типизираният pipeline
(добавен 2026-08-21) затваря тази дупка без да чупи съществуващ
callsite:

```cpp
// (1) Chain по коя да е колона, mutate, save (UPDATE)
User u = User::query<User>()
           ->where("email", email)
           ->where("active","=", true)
           ->firstOrFail();
u.set("last_login", now);
u.save();                          // UPDATE users SET ... WHERE id=<hydrated>

// (2) Зареждане по id
User u = User::findAs<User>(id);                     // throws ако липсва
auto  opt = User::tryFindAs<User>(id);               // std::optional<User>

// (3) Instance-ниво DELETE
User u = User::findAs<User>(id);
u.remove();                        // DELETE FROM users WHERE id=<hydrated>

// (4) Bulk UPDATE / DELETE — празен WHERE => throws

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

// (5) WHERE ... IS NULL на typed surface
Settings s = Settings::query<Settings>()
               ->where("user_id","IS", nullptr)
               ->firstOrFail();

// (6) List rehydration
std::vector<User> active = User::query<User>()
                             ->where("active","=", true)
                             ->get();
```

**Терминали:**

| Метод                        | Връща                      | При не-намерено |
| ---------------------------- | -------------------------- | --------------- |
| `first()`                    | `std::optional<T>`         | `nullopt`       |
| `firstOrFail()`              | `T`                        | throws          |
| `find(int id)`               | `std::optional<T>`         | `nullopt`       |
| `findOrFail(int id)`         | `T`                        | throws          |
| `get()`                      | `std::vector<T>`           | празен          |
| `update(json fields)`        | void                       | throws при празен WHERE |
| `remove()`                   | void                       | throws при празен WHERE |
| `Model::findAs<T>(id)`       | `T`                        | throws          |
| `Model::tryFindAs<T>(id)`    | `std::optional<T>`         | `nullopt`       |
| `Model::query<T>()`          | `std::unique_ptr<TypedQuery<T>>` | —         |

Setter-ите остават ръчни в наследниците (`this->set("email", v)`
вътре в scaffold-нат `UserMonet::setEmail(...)` helper); macro-
генерирани accessor-и не се въвеждат в тази ревизия.

**Guards.** Bulk `update()` / `remove()` отказват да работят без
поне един `where(...)` (защита срещу случайно full-table
пренаписване). `Model::remove()` върху нехидриран инстанс (празен
id) също хвърля.

**BC.** Класическият `Model::where<T>(...)` chain и нетипизираните
`json first() / find(int) / get()` терминали остават непроменени.

**Как работи вътрешно.** Connection layer-ите връщат резултата
като `JsonValue::RawJson` (JSON string). `TypedQuery` го парсва
обратно към Object/Array дърво през новия
`JsonValue::parse(std::string_view)` (RFC 8259, без dependencies) и
извиква `Model::hydrate(row)` — попълва `attributes` и сетва `id`,
така че следващ `save()` естествено попада в UPDATE-branch-a.

Затворени gap-ове от `vendors/Garvan/GARVAN.md`: **Gap 1** (Model
IS NULL — typed surface), **Gap 2** (DELETE — instance + bulk),
**Gap 6** (UPDATE с non-PK WHERE), **Gap 7** (rehydration в model
instance). Остават отворени: Gap 3 (aggregates), Gap 4 (JOIN
emission), Gap 5 (last-insert-id), Gap 8 (MonetDB PS bind
pipeline).

### C++23 ORM повърхнина

Освен typed pipeline vendor-ът излага цяло семейство C++23-специфични
API-та (всички паралелни на класическите — пълна BC е запазена). За
rebuild на vendor-а от source ти трябва **GCC 14+ или Clang 18+** (за
`deducing this` и `std::expected`).

**1. `ModelType` concept за static factory-та.**

```cpp
template <ModelType T>
static T* where(std::string field, std::string value);
// ModelType = std::derived_from<T, Garvan::Model>
//          && std::is_default_constructible_v<T>
```

Грешките при неправилен `T` са къси и четими вместо страници template
шум.

**2. Премахнат `delete this` — scratchpad ownership.**

Терминалните методи (`get`, `find`, `first`, …) вече не правят
`delete this`. Стек-инстанциите са безопасни. Моделите върнати от
static `where<T>()` живеят в `thread_local` scratchpad, който се
чисти при следващ `where<T>()` на същия thread или чрез явен:

```cpp
Garvan::Model::flushScratch();
```

**3. Typed WHERE overloads.**

```cpp
builder->where("id", "=", 42);                // int  -> "42"
builder->where("active", "=", true);          // bool -> "true"
builder->where("deleted_at", "IS", nullptr);  //      -> "NULL"
```

Concept-guarded overloads за `integral` / `floating_point` / `bool` /
`nullptr_t`. Съществуващият string API остава непокътнат.

**4. Deducing-`this` fluent value chain.**

```cpp
Builder b(...);
b.whereRef("id", "1").whereRef("age", ">", "18").get();
```

`whereRef` връща `Self&` вместо `Self*`, което позволява value
chain-и върху стек builder-и. Старият `Builder* where(...)` е
пазен за BC.

**5. `std::expected<json, DbError>` вместо exception-и.**

```cpp
auto result = user->tryFirst();
if (!result) {
    log(result.error().message);
    return;
}
json data = *result;
```

`DbError::Code = { Connection, Syntax, Constraint, NotFound, Unknown }`.
Класификацията в момента е евристична (по `what()`); прецизна
SQLSTATE класификация идва в бъдещо разширение на connection layer-а.
Throwing методите (`get`, `first`, `find*`) се пазят за BC.

**6. Хигиена.**

- `[[nodiscard]]` върху всички query terminals и getters.
- `std::string_view` overloads в `where`, `sanitizeOperator`,
  `sanitizeOrderBy`, `assertSafeIdentifier`.
- Operator allowlist е `constexpr std::array` — няма runtime heap
  allocation за static set.
- Include guards преименувани към `GARVAN_*`.
- `#include <pqxx/pqxx>` премахнат от `orm/omodel.h` (ORM-neutral
  header).

Виж `vendors/Garvan/README.md` за пълния vendor changelog.

### Kalpasan CLI

`kalpasan` е симлинк в основната директория към `vendors/Garvan/kalpasan` и
чете активния `.env` файл. Команди:

| Команда                                | Действие                                            |
| -------------------------------------- | --------------------------------------------------- |
| `./kalpasan make:model <Name>`         | Генерира `app/models/<Name>.{h,cpp}`                |
| `./kalpasan make:controller <Name>`    | Генерира `app/controllers/<Name>Controller.{h,cpp}` |
| `./kalpasan make:service <Name>`       | Генерира `app/services/<Name>Service.{h,cpp}`       |
| `./kalpasan make:migration <name>`     | Нов файл в `db/migrations/`                         |
| `./kalpasan db:migrate`                | Прилага миграциите                                  |
| `./kalpasan db:rollback`               | Връща последната миграция                           |
| `./kalpasan db:seed`                   | Пуска файлове от `db/seeders/`                      |
| `./kalpasan serve --watch`             | Билдва и рестартира при промяна на `.cpp` / `.h`    |
| `./kalpasan job:list`                  | `GET /api/admin/jobs` — списък регистрирани jobs    |
| `./kalpasan job:dispatch <Name> [--field k=v]` | `POST /api/admin/jobs/dispatch` — dispatch с JSON payload |
| `./kalpasan event:list`                | `GET /api/admin/events` — списък регистрирани events|
| `./kalpasan event:fire <Name> [--field k=v]`   | `POST /api/admin/events/fire` — fire с payload    |
| `./kalpasan route:list`                | Извежда HTTP routes                                 |
| `./kalpasan config:show` / `config:get` / `config:set` | Работа с `.env` стойности                   |
| `./kalpasan config:key:generate`       | Генерира нов `APP_KEY` (base64 32-byte)             |

Runtime verbs (`job:*`, `event:*`) са HTTP клиенти, които говорят с
работещото приложение по loopback. Четат `KALPASAN_ADMIN_URL`
(default `http://127.0.0.1:9090`) и `KALPASAN_ADMIN_TOKEN` от `.env`.
`AdminRoute` handler-ът иска и трите: непразен токен, loopback IP
(`127.0.0.1` или `::1`) и съвпадащ `Bearer` header — иначе връща
401/403/503 с JSON error body.

### Jobs & Events

Приложението boot-ва три service provider-а в `main.cpp` (редът има
значение — `AppServiceProvider` първи, после jobs, после events):

```
AppKernel::bootAll()
  ├── AppServiceProvider          (bind-ва queue driver-и)
  ├── JobServiceProvider          (регистрира job класове)
  └── EventServiceProvider        (регистрира events + listener-и)
```

End-to-end pipeline от sample event-а:

```
GET /api/events/user-registered?email=...&name=...&id=...
  → EventDispatcher::fire(UserRegistered)
      → LogRegistrationListener::handle()      (sync log ред)
      → SendWelcomeEmailListener::handle()
           → JobDispatcher::dispatch(SendTestMail)
                → SyncDriver (inline, същия thread)
                     → SendTestMail::handle()  (libcurl SMTP send)
```

**Създаване на нов job**

1. Файл `app/jobs/MyJob.{h,cpp}` наследяващ `Garvan::Job`; override
   `handle()`, `jobName()`, `payload()` и статичен фабричен
   `fromPayload(const Garvan::JsonValue&)`.
2. Регистрация в `app/providers/JobServiceProvider.cpp`:
   ```cpp
   Garvan::JobRegistry::bind("MyJob", &AppJobs::MyJob::fromPayload);
   ```
   ⚠️ **Не използвай** `GARVAN_REGISTER_JOB(AppJobs::MyJob)` за user
   jobs. Макросът stringify-ва целия token и регистрира ключа като
   `"AppJobs::MyJob"`, което не съвпада с `jobName()` (нито с
   short name-a, който `kalpasan job:dispatch` очаква). Виж
   pattern-a в `app/providers/JobServiceProvider.cpp:18`.
3. Rebuild (`./make.sh`) и рестарт на `./bin/app.bin`.

**Създаване на нов event + listener**

1. Event class в `app/events/` (наследява `Garvan::Event`).
2. Listener в `app/listeners/` (наследява
   `Garvan::Listener<YourEvent>`) с override на `handle()`.
3. Регистрация в `app/providers/EventServiceProvider.cpp`
   (`EventDispatcher::listen<...>()` + `GARVAN_REGISTER_EVENT(...)`).

### Mail конфигурация

`AppJobs::SendTestMail` праща реален HTML email по SMTP през libcurl.
Чете `MAIL_*` блока от `.env`:

```dotenv
MAIL_DRIVER=smtp
MAIL_HOST=smtp.example.com
MAIL_PORT=465                    # 465 -> implicit TLS (smtps://)
                                 # 587 -> STARTTLS   (smtp:// + CURLUSESSL_ALL)
MAIL_USERNAME="user@example.com"
MAIL_PASSWORD="app-password"
MAIL_ENCRYPTION=tls              # ssl / smtps -> форсира implicit TLS
MAIL_FROM_ADDRESS="user@example.com"
MAIL_FROM_NAME="Your App"
MAIL_AUTHENTICATION=plain        # informational; libcurl auto-negotiates
```

RFC 822 message-ът е `Content-Type: text/html; charset=UTF-8` с
8-bit CTE, така че `body` полето може да съдържа произволен HTML.

**Debug на SMTP dialog**

Ако send-ът тихо fail-ва или mail-ът отива в spam, временно включи
verbose в `app/jobs/SendTestMail.cpp`:

```cpp
// Set to 1L при debug на TLS handshake / SMTP dialog.
curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);   // ← смени 0L на 1L
```

Rebuild, рестарт, повтори опита и прочети целия TLS handshake +
SMTP dialog в stdout лога на сървъра. **Не commit-вай тази промяна** —
production лога трябва да е чист.

### Маршрути

Същият списък като в английската секция. JSON API е дефиниран в
`routes/ApiRoutes.cpp`, а уеб частта и документацията в `routes/WebRoutes.cpp`
и `routes/DocsRouter.cpp`.

### Примери с curl

```bash
# Списък потребители
curl http://localhost:9090/api/psql/users

# Създаване на потребител
curl -X POST http://localhost:9090/api/psql/users \
     -H "Content-Type: application/json" \
     -d '{"name":"Ada","email":"ada@example.com","password":"secret"}'

# Fire event → пълен listener chain → реален mail
curl "http://localhost:9090/api/events/user-registered?email=you@x.com&name=You&id=1"

# Директен job dispatch (bypass event bus) с HTML body
curl "http://localhost:9090/api/jobs/send-mail?to=you@x.com&subject=Hi&body=<h1>Hello</h1>"

# Debug endpoints
curl http://localhost:9090/api/jobs/status
curl http://localhost:9090/api/events/status

# Admin API (същия канал, който `kalpasan job:dispatch` ползва)
curl -X POST http://localhost:9090/api/admin/jobs/dispatch \
     -H 'Content-Type: application/json' \
     -H "Authorization: Bearer $KALPASAN_ADMIN_TOKEN" \
     -d '{"job":"SendTestMail","payload":{"to":"you@x.com","subject":"Hi","body":"<p>HTML</p>"}}'
```

### Вградена документация

Когато приложението работи, рамката сервира собствената си документация от
основния URL. Полезни начални точки:

- `/getting_started/setup_linux` — инсталация и билд.
- `/getting_started/your_first_application` — минимален Crow handler.
- `/guides/app`, `/guides/routes`, `/guides/middleware`, `/guides/websockets`.
- `/garvan/orm`, `/garvan/models`, `/garvan/migrations`, `/garvan/kalpasan`,
  `/garvan/databases`, `/garvan/env`.

Всички страници са налични на английски, български, испански, португалски,
руски и турски — превключете с `GET /lang/<code>`.

### Интернационализация (i18n)

Docs сайтът (и всяка consumer страница) се сервира през речниково-базиран
i18n pipeline. Всяка страница е **един канoничен mustache темплейт** с
`{{t_...}}` placeholder-и; реалният текст живее в JSON dict-ове за всеки
език, заредени при boot.

```
public/
├── langs/                     # един flat JSON dict на език
│   ├── en.json                # default + ultimate fallback
│   ├── bg.json
│   └── {ru,es,tr,pt}.json
└── pages/                     # 42 канонични темплейта (езиково-агностични)
    ├── home.html, license.html, privacy.html, reference.html
    ├── garvan/            (13 страници)
    ├── getting_started/   (6 страници)
    └── guides/            (20 страници)
```

**Компоненти:**

| Компонент                       | Файл                          | Роля                                                                       |
| ------------------------------- | ----------------------------- | -------------------------------------------------------------------------- |
| `AppServices::I18n`             | `app/services/I18n.{h,cpp}`   | Lazy-load-ва dict-овете, резолвва ключове с EN fallback, попълва mustache ctx. |
| `Routes::DocsRouter`            | `routes/DocsRouter.cpp`       | `PageMeta` index, route wiring, делегира превода на `I18n`.                |
| Crow mustache                   | (framework)                   | Рендерира `{{t_*}}` tokens спрямо инжектирания контекст.                   |
| `tools/extract_docs_i18n.py`    | `tools/`                      | One-shot миграционен helper (DocsRouter dict → JSON).                      |
| `tools/extract_pages_i18n.py`   | `tools/`                      | One-shot миграционен helper (per-lang HTML → JSON + deploy canonicals).    |

**Fallback верига:** `current lang → EN → литерално име на ключа`.

**Смяна на език:** `GET /lang/<code>` сетва 1-годишен `lang` cookie;
`AppServices::I18n::langCookieHeader(lang)` строи header стойността.

Пълен walkthrough с примери: виж [`/garvan/i18n`](/garvan/i18n) в
работещия docs сайт.

### Лиценз

GNU General Public License v3.0 — вижте [`LICENSE`](LICENSE).
