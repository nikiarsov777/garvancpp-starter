#include "TeamController.h"
#include "../../services/TeamService.h"

using namespace AppControllersApi;
using namespace AppServices;

TeamController::TeamController()
{

}

TeamController::~TeamController()
{

}

json TeamController::index()
{
    TeamService *teamService = new TeamService();
    return teamService->index();
}
