#ifndef USERMONGOCONTROLLER_H
#define USERMONGOCONTROLLER_H
#include <crow/http_request.h>
#pragma once

#include "controller/BaseContoller.h"
#include <bits/stdc++.h>
using namespace std;

namespace AppControllersApi
    {
    class UserMongoController
    {
    public:
        UserMongoController();
        ~UserMongoController();

        json index();

        json create(const crow::request& req);
    };
}

#endif // MONGOUSERCONTROLLER_H
