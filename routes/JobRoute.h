#ifndef JOB_ROUTE_H
#define JOB_ROUTE_H

#include "crow.h"
#include "tools/JsonValue.h"

#pragma once

using json = JsonValue;

namespace Routes
{
    // JobRoute -- HTTP endpoints за директен job dispatch (без
    // event bus в между). Полезно за smoke test на queue-a и за
    // ad-hoc admin действия.
    //
    //   GET /api/jobs/send-mail?to=x@y.z[&subject=...][&body=...]
    //     -> dispatch на SendTestMail през активния driver
    //
    //   GET /api/jobs/status
    //     -> {"drivers":[...], "registered_jobs":N}
    class JobRoute
    {
    public:
        JobRoute(crow::SimpleApp& app);
        ~JobRoute();
    };
}

#endif
