#include "TeamService.h"
#include "../models/Team.h"
#include "../models/User.h"

using namespace AppModels;
using namespace AppServices;

TeamService::TeamService()
{

}

TeamService::~TeamService()
{

}

json TeamService::index()
{
    Team *team = new Team();
    json result = team->where("id" ,"<", "5")->with(User())->get();

    return result;
}
