#include "UserMongoController.h"

#include "../../services/UserMongoService.h"
#include <bits/stdc++.h>

using namespace std;
using namespace AppControllersApi;
using namespace AppServices;

UserMongoController::UserMongoController() {}

UserMongoController::~UserMongoController()
{
    delete this;
}

json UserMongoController::index()
{
    cout << "UserMongoController::index" << endl;
    MongoUserService *userService = new MongoUserService();
    cout << "UserMongoController::index 222" << endl;
    return userService->index();
}

json UserMongoController::create(const crow::request &req)
{
    MongoUserService *userService = new MongoUserService();
    return userService->create(req);
}
