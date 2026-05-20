#ifndef USERMYSQLSERVICE_H
#define USERMYSQLSERVICE_H

#pragma once
// #include "crow.h"
#include "vendors/Garvan/crow.h"
#include "service/BaseService.h"


namespace AppServices
{
class UserMysqlService : public Garvan::BaseService
{
public:
    UserMysqlService();
    ~UserMysqlService();

    json index() override;
    json get(int id) {
        json err = json::Object();
        err["OK"] = 200;
        return err;
    };
    json posts(int id){
        json err = json::Object();
        err["OK"] = 200;
        return err;
    };
    json roles(int id){
        json err = json::Object();
        err["OK"] = 200;
        return err;
    };

    json create(const crow::request &req);

};
}

#endif // USERMYSQLSERVICE_H
