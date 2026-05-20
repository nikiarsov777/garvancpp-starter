#ifndef BUILDER_H
#define BUILDER_H

#include <string>
#include <vector>
#include "omodel.h"
#include "connection/db_connection.h"
#include "grammar/grammar.h"

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

    const json& getInsertData() const {
        return insertData;
    }

    const json& getUpdateData() const {
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
    json executeQuery()
    {
        if (!grammar) {
            std::cout << "GRAMMAR IS NULL!" << std::endl;
            return json::Array();
        }
        PreparedStatement ps = grammar->compileSelect(*this);
        return dbClient->execute(ps);
    }

    json executeWrite()
    {
        if (!grammar) {
            std::cout << "GRAMMAR IS NULL!" << std::endl;
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

    json get()
    {
        return executeQuery();
    }

    json get(vector<string> selectedColumns)
    {
        this->public_columns = selectedColumns;
        return get();
    }

    json first()
    {
        this->limit = 1;
        return executeQuery();
    }

    json firstOrFail()
    {
        auto result = first();
        if (result.empty())
            throw runtime_error("Record not found");
        return result;
    }

    json find(int id)
    {
        where(primaryKey, "=", to_string(id));
        return first();
    }

    json findOrFail(int id)
    {
        auto result = find(id);
        if (result.empty())
            throw runtime_error("Record not found");
        return result;
    }

    // =======================
    // WHERE
    // =======================

    Builder *where(string field, string value)
    {
        wheres.push_back({field, "=", value});
        return this;
    }

    Builder *where(string field, string op, string value)
    {
        wheres.push_back({field, op, value});
        return this;
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

    const string &getTable() const { return table; }
    const vector<WhereClause> &getWheres() const { return wheres; }
    const string &getOrder() const { return order_by; }
    int getLimit() const { return limit; }
    int getOffset() const { return offset; }
    const vector<string> &getColumns() const { return public_columns; }

    // =======================
    // RESET
    // =======================

    void reset()
    {
        wheres.clear();
        limit = 10;
        offset = 0;
    }
};
}

#endif // BUILDER_H
