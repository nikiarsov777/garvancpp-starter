#include "UserMysqlController.h"

#include "../../services/UserMysqlService.h"
#include <bits/stdc++.h>

using namespace std;
using namespace AppControllersApi;
using namespace AppServices;

UserMysqlController::UserMysqlController() {}

UserMysqlController::~UserMysqlController()
{

}

json UserMysqlController::index()
{
    UserMysqlService *userService = new UserMysqlService();
    return userService->index();
}

json UserMysqlController::create(const crow::request &req)
{
    UserMysqlService *userService = new UserMysqlService();
    return userService->create(req);
}
