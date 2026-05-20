#include "UserMongo.h"

namespace AppModels {

// UserMongo::UserMongo(const char *customDriver) : BaseModel("MONGODB")
// {
//     table = "users";
//     public_columns = {"id", "name" , "email", "created_at", "updated_at", "bcol", "money", "fcol", "jcol"};
//     private_columns = {"email_verified_at", "password"};

//     init();
// }

UserMongo::UserMongo(): BaseModel("MONGODB")
{
    table = "users";
    public_columns = {"id", "name" , "email", "created_at", "updated_at", "bcol", "money", "fcol", "jcol"};
    private_columns = {"email_verified_at", "password"};

    init();
}

}
