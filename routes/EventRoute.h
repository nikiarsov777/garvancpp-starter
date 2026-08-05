#ifndef EVENT_ROUTE_H
#define EVENT_ROUTE_H

#include "crow.h"
#include "tools/JsonValue.h"

#pragma once

using json = JsonValue;

namespace Routes
{
    // EventRoute -- HTTP endpoints за event firing. Всеки endpoint
    // конструира съответния event и извиква `EventDispatcher::fire`.
    // Регистрираните listener-и (виж EventServiceProvider) реагират.
    //
    //   GET /api/events/user-registered?email=x@y.z[&name=...][&id=N]
    //     -> fire на UserRegistered event
    //     -> LogRegistrationListener  (sync, prints line)
    //     -> SendWelcomeEmailListener (sync -> dispatch SendTestMail)
    class EventRoute
    {
    public:
        EventRoute(crow::SimpleApp& app);
        ~EventRoute();
    };
}

#endif
