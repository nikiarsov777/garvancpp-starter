#ifndef USERPSQLSERVICE_H
#define USERPSQLSERVICE_H

#pragma once
// #include "crow.h"
#include "vendors/Garvan/crow.h"
#include "service/BaseService.h"


namespace AppServices
{
class UserPsqlService : public Garvan::BaseService
    {
    public:
        UserPsqlService();
        ~UserPsqlService();

        json index() override;
        json get(int id);
        json posts(int id);
        json roles(int id);

        json create(const crow::request &req);

    private:
    protected:
    };
}

#endif
