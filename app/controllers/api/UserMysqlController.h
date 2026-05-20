#ifndef USERMYSQLCONTROLLER_H
#define USERMYSQLCONTROLLER_H

#include <crow/http_request.h>
#pragma once

#include "controller/BaseContoller.h"
#include <bits/stdc++.h>
using namespace std;

namespace AppControllersApi
{
class UserMysqlController : public Garvan::BaseContoller
{
public:
    UserMysqlController();
    ~UserMysqlController();

    json index();
    json get(int id);
    json posts(int id);
    json roles(int id);

    json create(const crow::request& req);
};
}

#endif // USERMYSQLCONTROLLER_H
