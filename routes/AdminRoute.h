#ifndef ADMIN_ROUTE_H
#define ADMIN_ROUTE_H

#include "crow.h"
#include "tools/JsonValue.h"

#pragma once

using json = JsonValue;

namespace Routes
{
    // AdminRoute -- защитени HTTP endpoints за kalpasan CLI.
    //
    //   GET  /api/admin/jobs
    //   POST /api/admin/jobs/dispatch      body: {"job":"Name","payload":{...}}
    //   GET  /api/admin/events
    //   POST /api/admin/events/fire        body: {"event":"Name","payload":{...}}
    //
    // Auth:
    //   * Ако `KALPASAN_ADMIN_TOKEN` в .env е празно/липсва:
    //     всеки admin endpoint връща 503 (admin disabled) -- fail-secure.
    //   * Client IP != 127.0.0.1 → 403 (loopback-only).
    //   * Missing / грешен Bearer → 401.
    class AdminRoute
    {
    public:
        AdminRoute(crow::SimpleApp& app);
        ~AdminRoute();
    };
}

#endif
