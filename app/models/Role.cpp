#include "Role.h"

using namespace AppModels;

Role::Role(): BaseModel("PSQL")
{
    table = "roles";
    public_columns = {"id", "name"};
    private_columns = {};

    init();
}

Role::~Role()
{
}
