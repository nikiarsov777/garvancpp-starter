#include "PostController.h"
#include "../../services/PostService.h"
#include <bits/stdc++.h>

using namespace std;
using namespace AppControllersApi;
using namespace AppServices;


PostController::PostController()
{

}

PostController::~PostController()
{

}

json AppControllersApi::PostController::index()
{
    PostService *postService = new PostService();
    return postService->index();
}
