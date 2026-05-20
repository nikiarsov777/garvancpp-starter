#ifndef APP_CONTROLLERS_UserController_H
#define APP_CONTROLLERS_UserController_H

#pragma once

#include <crow/http_request.h>
#include "controller/BaseContoller.h"

namespace AppControllers
{

class UserController : public Garvan::BaseContoller
{
public:
    UserController();
    ~UserController();

    json index();
    json get(int id);
    json create(const crow::request& req);
    json update(int id, const crow::request& req);
    json destroy(int id);
};

} // namespace AppControllers

#endif
