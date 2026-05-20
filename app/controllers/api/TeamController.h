#ifndef TEAMCONTROLLER_H
#define TEAMCONTROLLER_H

#pragma once

#include "controller/BaseContoller.h"
#include <bits/stdc++.h>
using namespace std;

namespace AppControllersApi
{
    class TeamController  : public Garvan::BaseContoller
    {
    public:
        TeamController();
        ~TeamController();

        json index();

    private:
    };
}

#endif
