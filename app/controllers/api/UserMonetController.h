#ifndef USERMONETCONTROLLER_H
#define USERMONETCONTROLLER_H
#include <crow/http_request.h>
#pragma once

#include "controller/BaseContoller.h"
#include <bits/stdc++.h>
using namespace std;

namespace AppControllersApi
{
class UserMonetController
{
public:
    UserMonetController();
    ~UserMonetController();
    json index();

    json create(const crow::request& req);
};
}

#endif // USERMONETCONTROLLER_H
