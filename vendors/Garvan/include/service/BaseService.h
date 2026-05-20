#ifndef BASESERVICE_H
#define BASESERVICE_H

#pragma once

#include <pqxx/pqxx>

#include "../tools/Helper.h"
#include "../crow.h"


using namespace std;

namespace Garvan
{
    class BaseService : public Helper
    {
    public:
        BaseService();
        virtual ~BaseService();
        
        virtual json index();
        virtual json create(json model);
        json create(const crow::request &req);
        virtual json read(string id);
        virtual json update(json model);
        virtual void erease();
        virtual json getById(string id);
        
        
    protected:
        
    private:
    };
}

#endif
