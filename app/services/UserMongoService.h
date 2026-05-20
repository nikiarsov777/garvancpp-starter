#ifndef USERMONGOSERVICE_H
#define USERMONGOSERVICE_H

#include "vendors/Garvan/crow.h"
#include "service/BaseService.h"


namespace AppServices
{
class MongoUserService : public Garvan::BaseService
{
public:
    MongoUserService();

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

#endif // USERMONGOSERVICE_H
