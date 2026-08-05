#include "JobRoute.h"

#include <memory>
#include <string>

#include "queue/JobDispatcher.h"
#include "queue/JobRegistry.h"

#include "../app/jobs/SendTestMail.h"

using namespace Routes;

namespace {

// Локален helper struct — same pattern като в ApiRoutes.cpp.
// Задава CORS + JSON headers веднъж, за да не се дублира кода в
// всеки CROW_ROUTE lambda.
struct jsonresponse : crow::response
{
    jsonresponse(const json& body) : crow::response{body.dump()}
    {
        add_header("Access-Control-Allow-Origin", "*");
        add_header("Access-Control-Allow-Headers", "Content-Type");
        add_header("Content-Type", "application/json");
    }
};

} // namespace

JobRoute::JobRoute(crow::SimpleApp& app)
{
    // ---------------------------------------------------------------
    // GET /api/jobs/send-mail?to=X[&subject=Y][&body=Z]
    //
    // Директен job dispatch — прескача event bus-a. Job-ът минава
    // през JobDispatcher::dispatch, което рутира на база
    // `job->connection()` (Phase A: винаги "sync" → SyncDriver го
    // изпълнява в този request thread преди да върнем response).
    // ---------------------------------------------------------------
    CROW_ROUTE(app, "/api/jobs/send-mail")
    ([](const crow::request& req) {
        // Всички query параметри са optional — endpoint-ът е замислен
        // и като smoke test (curl без args), и като real dispatch с
        // ?to=...&subject=...&body=... Дефолтите гарантират, че поне
        // един цикъл през JobDispatcher / SyncDriver / SendTestMail
        // винаги минава — вижда се в stdout log-а.
        std::string to      = req.url_params.get("to")      ? req.url_params.get("to")      : "demo@bgchess.zone";
        std::string subject = req.url_params.get("subject") ? req.url_params.get("subject") : "Test mail from Garvan";
        std::string body    = req.url_params.get("body")    ? req.url_params.get("body")    : "Hello from JobRoute::send-mail";

        try {
            Garvan::JobDispatcher::dispatch(
                std::make_unique<AppJobs::SendTestMail>(to, subject, body));
        } catch (const std::exception& e) {
            json err = json::Object();
            err["status"] = "error";
            err["error"]  = e.what();
            return jsonresponse{err};
        }

        json ok = json::Object();
        ok["status"] = "dispatched";
        ok["job"]    = "SendTestMail";
        ok["to"]     = to;
        ok["subject"]= subject;
        return jsonresponse{ok};
    });

    // ---------------------------------------------------------------
    // GET /api/jobs/status
    //
    // Debug endpoint — показва кои driver-и са bound-нати в
    // JobDispatcher и колко Job класа са в JobRegistry.
    // ---------------------------------------------------------------
    CROW_ROUTE(app, "/api/jobs/status")
    ([]() {
        json out = json::Object();
        auto drivers = json::Array();
        for (const char* name : {"sync", "async", "database"}) {
            if (Garvan::JobDispatcher::driver(name) != nullptr) {
                drivers.push_back(std::string(name));
            }
        }
        out["drivers"]         = drivers;
        out["registered_jobs"] = static_cast<int>(Garvan::JobRegistry::size());
        return jsonresponse{out};
    });
}

JobRoute::~JobRoute() = default;
