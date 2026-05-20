#include "Comment.h"

using namespace AppModels;

Comment::Comment(): BaseModel("PSQL")
{
    table = "comments";
    public_columns = {"id", "user_id", "post_id" , "comment",  "created_at", "updated_at"};
    private_columns = {};

    init();
}

Comment::~Comment()
{

}
