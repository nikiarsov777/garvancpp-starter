#include "UserPsqlController.h"
#include "../../services/UserPsqlService.h"
#include <bits/stdc++.h>

using namespace std;
using namespace AppControllersApi;
using namespace AppServices;

UserPsqlController::UserPsqlController()
{
}

UserPsqlController::~UserPsqlController()
{
}

json UserPsqlController::index()
{
    UserPsqlService *userService = new UserPsqlService();
    return userService->index();
}

json UserPsqlController::create(const crow::request &req)
{
    UserPsqlService *userService = new UserPsqlService();
    return userService->create(req);
}

json AppControllersApi::UserPsqlController::get(int id)
{
    UserPsqlService *userService = new UserPsqlService();
    return userService->get(id);
}

json AppControllersApi::UserPsqlController::posts(int id)
{
    UserPsqlService *userService = new UserPsqlService();
    return userService->posts(id);
}

json AppControllersApi::UserPsqlController::roles(int id)
{
    UserPsqlService *userService = new UserPsqlService();
    return userService->roles(id);
}
