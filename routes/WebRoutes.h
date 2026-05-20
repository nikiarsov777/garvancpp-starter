#ifndef WEBROUTES_H
#define WEBROUTES_H

#include "crow.h"
#include "tools/JsonValue.h"

#pragma once

using json = JsonValue;

namespace Routes
{
    class WebRoutes
    {
    public:
        WebRoutes(crow::SimpleApp &app);
        ~WebRoutes();
        void test(){}

    private:
    };
}

#endif
