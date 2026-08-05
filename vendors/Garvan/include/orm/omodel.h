#ifndef GARVAN_OMODEL_H
#define GARVAN_OMODEL_H
// NOTE: <pqxx/pqxx> се изнесе от този header — това е ORM-neutral base
// клас и Postgres driver-ът няма работа тук. Файловете, които реално
// имат нужда от pqxx (например `orm/connection/pg_connection.cpp`),
// го инклудват директно.
#include "../db/DbClient.h"

using namespace std;

namespace ORM
{

    class OModel
    {
    public:
        OModel();
        virtual ~OModel();

        string getTable();
        string setTable(string table);
        string getPimaryKey();
        vector<string> getPublicColumns() const;
        vector<string> getPrivateColumns() const;

        void setId(string id);
        string getId();

    protected:
        string table = "example";
        string primaryKey = "id";
        string columns = "*";
        string order_by = "1 asc";
        int limit = 10;
        int offset = 0;
        // string query = "";
        // string whr = "";
        // string joinColumns = "";
        // string joinTable = "";

        vector<string> public_columns = {"id"};
        vector<string> private_columns = {"id"};

    private:
        string id;
    };
}
#endif // GARVAN_OMODEL_H
