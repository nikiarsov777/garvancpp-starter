#ifndef TEAMSERVICE_H
#define TEAMSERVICE_H

#pragma once

#include "service/BaseService.h"

namespace AppServices
{
    class TeamService : public Garvan::BaseService
    {
    public:
        TeamService();
        ~TeamService();

        json index() override;

    private:
    };
}

#endif
