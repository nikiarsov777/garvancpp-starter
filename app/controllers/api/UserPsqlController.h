#ifndef USERPSQLCONTROLLER_H
#define USERPSQLCONTROLLER_H

#include <crow/http_request.h>
#pragma once

#include "controller/BaseContoller.h"
#include <bits/stdc++.h>
using namespace std;

namespace AppControllersApi
{
    class UserPsqlController : public Garvan::BaseContoller
    {
    public:
        UserPsqlController();
        ~UserPsqlController();

        json index();
        json get(int id);
        json posts(int id);
        json roles(int id);

        json create(const crow::request& req);

    private:
    };
}

#endif
