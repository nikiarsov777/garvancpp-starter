#include "RoleService.h"
#include "../models/Role.h"
#include "../models/User.h"

using namespace AppModels;
using namespace AppServices;

RoleService::RoleService()
{

}

RoleService::~RoleService()
{

}

json RoleService::index()
{
    Role * role = new Role();
    json result = role->belongsTo(User(), "role_id", "id")->get();
    return result;
}
