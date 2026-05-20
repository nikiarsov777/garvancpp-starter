#include "PostService.h"
#include "../models/Post.h"
#include "../models/Comment.h"

using namespace AppModels;
using namespace AppServices;

PostService::PostService()
{

}

PostService::~PostService()
{

}

json AppServices::PostService::index()
{
    Post *user = new Post();
    // json result = user->hasMany(Comment())->get();

    Comment *comment = new Comment();
    json result = comment->belongsTo(Post())->get();

    return result;
}
