#include "Team.h"

using namespace AppModels;

Team::Team(): BaseModel("PSQL")
{
    table = "teams";
    public_columns = {"id", "name"};
    private_columns = {};

    init();
}

Team::~Team()
{
}
