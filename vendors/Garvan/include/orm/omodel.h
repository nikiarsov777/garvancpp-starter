#ifndef MODEL_H
#define MODEL_H
#include <pqxx/pqxx>
#include <pqxx/field>
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
#endif // MODEL_H
