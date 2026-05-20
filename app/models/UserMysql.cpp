#include "UserMysql.h"

namespace AppModels {

// UserMysql::UserMysql(const char *customDriver) : BaseModel("MONGODB")
// {
//     table = "users";
//     public_columns = {"id", "name" , "email", "created_at", "updated_at", "bcol", "money", "fcol", "jcol"};
//     private_columns = {"email_verified_at", "password"};

//     init();
// }

UserMysql::UserMysql(): BaseModel("MYSQL")
{
    table = "users";
    public_columns = {"id", "name" , "email", "created_at", "updated_at", "bcol", "money", "fcol", "jcol"};
    private_columns = {"email_verified_at", "password"};

    init();
}

}
