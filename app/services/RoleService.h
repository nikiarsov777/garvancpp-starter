#ifndef ROLESERVICE_H
#define ROLESERVICE_H

#pragma once

#include "service/BaseService.h"

namespace AppServices
{

    class RoleService : public Garvan::BaseService
    {
    public:
        RoleService();
        ~RoleService();

        json index() override;

    private:
    };
}

#endif
