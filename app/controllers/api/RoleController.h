#ifndef ROLECONTROLLER_H
#define ROLECONTROLLER_H

#pragma once

#include "controller/BaseContoller.h"

namespace AppControllersApi
{

    class RoleController : public Garvan::BaseContoller
    {
    public:
        RoleController();
        ~RoleController();

        json index();

    private:
    };
}

#endif
