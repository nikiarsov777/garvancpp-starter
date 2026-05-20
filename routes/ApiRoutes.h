#ifndef APIROUTES_H
#define APIROUTES_H

#include "crow.h"
#include "tools/JsonValue.h"

#pragma once

using json = JsonValue;

namespace Routes
{
    class ApiRoutes
    {
    public:
        ApiRoutes(crow::SimpleApp &app);
        ~ApiRoutes();

    private:
    };
}

#endif
