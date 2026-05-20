#ifndef GARVAN_MIGRATE_MIGRATIONRUNNER_H
#define GARVAN_MIGRATE_MIGRATIONRUNNER_H

#pragma once

#include "Migration.h"
#include "MigrationStore.h"

#include <memory>
#include <string>
#include <vector>

namespace Garvan
{
class DbClient;

// Top-level orchestrator. Constructed once per `garvan-migrate <cmd>`
// invocation. Owns the DbClient (constructed from the requested
// .env prefix) and a MigrationStore for the matching backend.
class MigrationRunner
{
public:
    struct Options {
        // .env prefix that selects which <PREFIX>_DATABASE_* group to
        // load. When `clientExplicit` is false this is overridden by
        // the MIGRATION_DB value from .env (or falls back to "PSQL").
        std::string clientPrefix   = "";
        bool        clientExplicit = false;

        std::string migrationsDir  = "db/migrations";
        std::string schemaDir      = "db/schema";
    };

    explicit MigrationRunner(Options opts);
    // Out-of-line so unique_ptr<DbClient> can see the full type in the
    // .cpp without dragging DbClient.h into every includer.
    ~MigrationRunner();

    // Top-level commands. Each returns the process exit code (0 = OK).
    int cmdNew(const std::string& name);
    int cmdUp();
    int cmdDown();
    int cmdReset();   // revert every applied migration (artisan migrate:reset)
    int cmdFresh();   // drop + create + up (artisan migrate:fresh)
    int cmdStatus();
    int cmdCreate();
    int cmdDrop();
    int cmdDump();
    int cmdLoad();

    const std::string& dbtype() const { return dbtype_; }

private:
    Options                       opts_;
    std::string                   dbtype_;       // postgres / mysql / ...
    std::unique_ptr<DbClient>     db_;
    std::unique_ptr<MigrationStore> store_;

    // Discover all migration files for the active backend, sorted by
    // version ascending. Files for other backends are silently
    // skipped.
    std::vector<Migration> discover() const;

    // SQL backends: execute one statement at a time.
    // Mongo: each "statement" is a runCommand JSON document.
    void applyOne(const Migration& m);
    void revertOne(const Migration& m);

    // Filename suffix that selects this backend, e.g.
    //   postgres -> ".postgres.sql"
    //   mongodb  -> ".mongodb.json"
    std::string fileSuffix() const;
    std::string fileExtension() const; // ".sql" or ".json"

    void connectDb();
};

} // namespace Garvan

#endif
