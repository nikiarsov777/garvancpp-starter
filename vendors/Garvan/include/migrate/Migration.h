#ifndef GARVAN_MIGRATE_MIGRATION_H
#define GARVAN_MIGRATE_MIGRATION_H

#pragma once

#include <string>
#include <vector>

namespace Garvan
{

// One migration file on disk. The runner builds a list of these,
// orders them by `version`, and asks each one for its up/down
// statements when it needs to apply or revert.
//
// Filename convention (always .sql so editors / git filetype maps
// behave uniformly across backends):
//   default client:    <version>.sql
//   explicit client:   <version>.<dbtype>.sql
// where <version> is "<UTC YYYYMMDDhhmmss>_<descriptive_name>".
//
// All files use the dbmate-compatible section markers:
//   -- migrate:up
//     ...content...
//   -- migrate:down
//     ...content...
//
// SQL backends:
//     ...content... is one or more SQL statements separated by `;`.
//
// MongoDB:
//     ...content... is zero or more top-level balanced `{...}` JSON
//     documents. Each one is sent verbatim to db.runCommand(), in
//     declaration order. Anything outside an object — whitespace,
//     blank lines, -- comments — is ignored.
class Migration
{
public:
    enum class Kind { Sql, MongoJson };

    Migration() = default;

    // Parse `path`. dbtype is used only for diagnostics.
    static Migration load(const std::string& path,
                          const std::string& dbtype);

    const std::string& version()  const { return version_; }
    const std::string& filename() const { return filename_; }
    const std::string& path()     const { return path_; }
    Kind               kind()     const { return kind_; }

    // For SQL migrations: each entry is a single statement (already
    // split, comments stripped). For Mongo migrations: each entry is
    // a JSON document (one command) to be sent via runCommand.
    const std::vector<std::string>& upStatements()   const { return up_; }
    const std::vector<std::string>& downStatements() const { return down_; }

    // Public statement splitters, exposed so other tooling (seeders,
    // ad-hoc executors, ...) can share the exact same parsing rules
    // as the migration runner.
    static std::vector<std::string> splitSql(const std::string& body);
    static std::vector<std::string> splitMongoJson(const std::string& body);

private:
    std::string version_;
    std::string filename_;
    std::string path_;
    Kind        kind_ = Kind::Sql;

    std::vector<std::string> up_;
    std::vector<std::string> down_;
};

} // namespace Garvan

#endif
