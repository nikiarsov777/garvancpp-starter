#include "UserController.h"

using namespace AppControllers;

UserController::UserController()  {}
UserController::~UserController() {}

json UserController::index()
{
    json out = json::Object();
    out["data"] = json::Array();
    return out;
}

json UserController::get(int id)
{
    json out = json::Object();
    out["data"] = json::Object();
    out["data"]["id"] = id;
    return out;
}

json UserController::create(const crow::request& req)
{
    json out = json::Object();
    out["data"] = json::Object();
    return out;
}

json UserController::update(int id, const crow::request& req)
{
    json out = json::Object();
    out["data"] = json::Object();
    out["data"]["id"] = id;
    return out;
}

json UserController::destroy(int id)
{
    json out = json::Object();
    out["deleted"] = id;
    return out;
}
