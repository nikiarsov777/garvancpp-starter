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

### Project structure

```
.
├── app/
│   ├── controllers/        # web controllers
│   │   └── api/            # JSON API controllers (psql, mysql, mongo, monet)
│   ├── models/             # User, Role, Team, Post, Comment, ...
│   └── services/           # business logic per backend
├── routes/
│   ├── ApiRoutes.cpp       # /api/* endpoints
│   ├── WebRoutes.cpp       # / and language switch
│   └── DocsRouter.cpp      # multi-language docs router
├── db/
│   ├── migrations/         # *.<backend>.sql files
│   └── seeders/            # *.sql seeders run by kalpasan
├── public/                 # mustache templates + multi-language docs pages
├── static/                 # css, js, images
├── vendors/Garvan/         # libgarvan.a, crow headers, kalpasan, garvan-migrate
├── backs/                  # plain g++ fallback build scripts
├── main.cpp                # entry point — boots Crow on port 9090
├── CMakeLists.txt          # primary build
└── .env                    # runtime configuration
```

The HTTP server is started in `main.cpp:16` and listens on **port 9090** by default.

### Requirements

- GCC 11+ or Clang 12+ (C++20).
- CMake 3.20 or later.
- Asio 1.28+ development headers.
- Database client libraries: `libpqxx`, `libmysqlcppconn`, `libsqlite3`,
  `libmongoc` / `libbson` (mongocxx), `libmonetdb-mapi`.
- Optional: OpenSSL (HTTPS) and zlib (compression).

Debian / Ubuntu one-liner:

```bash
sudo apt update
sudo apt install -y \
    build-essential cmake \
    libasio-dev libssl-dev zlib1g-dev \
    libpqxx-dev libmysqlcppconn-dev libsqlite3-dev \
    libmongoc-dev libbson-dev
```

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
git clone https://github.com/nikiarsov777/garvancpp-orm-pub.git
cd garvancpp-orm-pub
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

Copy the freshly built `libgarvan.a` (and any updated headers under `include/`)
over the files in your project's `vendors/Garvan/` directory, then rebuild the
app with `cmake --build build -j` or `./make.sh`.

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

### Структура на проекта

Същата структура като в английската секция по-горе. HTTP сървърът се стартира
в `main.cpp:16` на **порт 9090** по подразбиране.

### Изисквания

- GCC 11+ или Clang 12+ с поддръжка на C++20.
- CMake 3.20 или по-нов.
- Asio 1.28+ development хедъри.
- Драйвери: `libpqxx`, `libmysqlcppconn`, `libsqlite3`, `libmongoc` /
  `libbson` (mongocxx), `libmonetdb-mapi`.
- По избор: OpenSSL за HTTPS и zlib за компресия.

За Debian / Ubuntu използвайте същата `apt install` команда от английската
секция.

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
git clone https://github.com/nikiarsov777/garvancpp-orm-pub.git
cd garvancpp-orm-pub
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

Копирайте новопостроения `libgarvan.a` (и евентуално обновените хедъри от
`include/`) върху файловете в `vendors/Garvan/` на проекта и пребилдвайте
приложението с `cmake --build build -j` или `./make.sh`.

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

### Лиценз

GNU General Public License v3.0 — вижте [`LICENSE`](LICENSE).
