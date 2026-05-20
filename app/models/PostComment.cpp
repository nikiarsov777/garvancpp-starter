#include "PostComment.h"

using namespace AppModels;

PostComment::PostComment(): BaseModel("PSQL")
{
    table = "fblog_comments";
    public_columns = {"id", "user_id" , "post_id", "comment", "created_at", "updated_at"};
    private_columns = {"approved", "approved_at"};

    init();
}

PostComment::~PostComment()
{

}
