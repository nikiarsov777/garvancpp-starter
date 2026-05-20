#ifndef USERMONETSERVICE_H
#define USERMONETSERVICE_H

#include "vendors/Garvan/crow.h"
#include "service/BaseService.h"


namespace AppServices
{
class UserMonetService : public Garvan::BaseService
{
public:
    UserMonetService();

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

#endif // USERMONETSERVICE_H
