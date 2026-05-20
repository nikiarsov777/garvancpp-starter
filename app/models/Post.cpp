#include "Post.h"

using namespace AppModels;

Post::Post(): BaseModel("PSQL")
{
    table = "posts";
    public_columns = {"id", "user_id", "title" , "slug", "sub_title", "body", "status",  "created_at", "updated_at"};
    private_columns = {};

    init();
}

Post::~Post()
{

}
