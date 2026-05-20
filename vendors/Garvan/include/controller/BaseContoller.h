#ifndef BASECONTOLLER_H
#define BASECONTOLLER_H

#pragma once

#include <pqxx/pqxx>

#include "../tools/Helper.h"

using namespace std;

namespace Garvan
{
    
class BaseContoller : public Helper
{
public:
    BaseContoller();
    virtual ~BaseContoller();

    virtual json index() = 0;
    json create(json model);
    json read(string id);
    json update(json model);
    void erease();
    json getById(string id);

protected:
    
private:
};
}
#endif
