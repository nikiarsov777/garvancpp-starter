#include "UserMonet.h"

namespace AppModels {
UserMonet::UserMonet() : BaseModel("MONETDB")
{
    table = "users";
    public_columns = {"id", "name" , "email", "created_at", "bcol", "money", "fcol", "jcol"};
    private_columns = {"email_verified_at", "password"};

    init();
}
}
