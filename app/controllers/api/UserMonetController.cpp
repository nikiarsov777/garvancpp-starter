#include "UserMonetController.h"
#include "../../services/UserMonetService.h"
#include <bits/stdc++.h>

using namespace std;
using namespace AppControllersApi;
using namespace AppServices;

UserMonetController::UserMonetController() {}
UserMonetController::~UserMonetController()
{
    delete this;
}

json UserMonetController::index()
{
    cout << "UserMonetController::index" << endl;
    UserMonetService *userService = new UserMonetService();
    cout << "UserMonetService::index 222" << endl;
    return userService->index();
}

json UserMonetController::create(const crow::request &req)
{
    UserMonetService *userService = new UserMonetService();
    return userService->create(req);
}
