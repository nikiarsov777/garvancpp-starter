#ifndef GARVAN_MIGRATE_MIGRATIONSTORE_H
#define GARVAN_MIGRATE_MIGRATIONSTORE_H

#pragma once

#include <string>
#include <vector>

namespace Garvan
{
class DbClient;

// Per-backend ledger of applied migrations. Backed by a SQL table
// `schema_migrations(version PRIMARY KEY)` for SQL connections, and a
// `schema_migrations` collection with `{_id: version}` for Mongo.
//
// The store lazily creates the underlying object on first use, so
// `applied()` is safe to call against a fresh database.
class MigrationStore
{
public:
    // dbtype is the value of <PREFIX>_DATABASE_TYPE env var:
    //   "postgres", "mysql", "sqlite", "monetdb", "mongodb".
    MigrationStore(DbClient* db, std::string dbtype);

    void ensureExists();

    std::vector<std::string> applied();          // versions, ascending
    bool                     isApplied(const std::string& version);

    void                     markApplied(const std::string& version);
    void                     markReverted(const std::string& version);

private:
    DbClient*   db_;
    std::string dbtype_;

    bool isMongo() const { return dbtype_ == "mongodb"; }

    void ensureSqlTable();
    void ensureMongoCollection();
};

} // namespace Garvan

#endif
