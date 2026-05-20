#ifndef DBCLIENT_H
#define DBCLIENT_H

#include <iostream>
#include <string>
#include "../tools/Helper.h"
#include "../orm/db_factory.h"
#include <mysql_driver.h>


using namespace std;
namespace Garvan
{
    class DbClient
    {
        
    private:
        Grammar* grammar;
        Helper helper;
        string dbType;
        
    protected:
        
    public:
        DbClient(string client);
        ~DbClient();
        json execute(string query);
        json execute(const PreparedStatement& stmt);
        DbFactory * dbFactory;

        Grammar* getGrammar();
    };
}
#endif
