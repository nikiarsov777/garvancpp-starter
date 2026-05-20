#include "RoleController.h"
#include "../../services/RoleService.h"
#include <bits/stdc++.h>

using namespace std;
using namespace AppControllersApi;
using namespace AppServices;

RoleController::RoleController()
{

}

RoleController::~RoleController()
{

}

json RoleController::index()
{
    RoleService *roleService = new RoleService();
    return roleService->index();
}