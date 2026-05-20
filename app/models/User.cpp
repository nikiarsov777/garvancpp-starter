#include "User.h"
#include "Post.h"

using namespace AppModels;

User::User(): BaseModel("PSQL")
{
    table = "users";
    public_columns = {"id", "name" , "email", "created_at", "updated_at", "bcol", "money", "fcol", "jcol"};
    private_columns = {"email_verified_at", "password"};

    init();
}

User *AppModels::User::posts()
{
    return dynamic_cast<User *>(this->hasMany(Post()));
}

User::~User()
{
    // dtorlL
}

