#include "AdminRoute.h"

#include <memory>
#include <string>
#include <string_view>

#include "tools/Helper.h"

#include "queue/JobDispatcher.h"
#include "queue/JobRegistry.h"
#include "events/EventDispatcher.h"
#include "events/EventRegistry.h"

using namespace Routes;

namespace {

// ------------------------------------------------------------
// Response helper — same pattern като ApiRoutes.cpp.
// ------------------------------------------------------------
struct jsonresponse : crow::response
{
    jsonresponse(int status_, const json& body) : crow::response{status_, body.dump()}
    {
        add_header("Access-Control-Allow-Origin", "*");
        add_header("Content-Type", "application/json");
    }
    jsonresponse(const json& body) : jsonresponse{200, body} {}
};

// ------------------------------------------------------------
// Trim `"..."` around .env values (Garvan::Helper::getenv не
// сваля кавичките; вижте MAIL_FROM_ADDRESS="info@..." в .env).
// ------------------------------------------------------------
std::string trimQuotes(std::string s)
{
    if (s.size() >= 2
        && ((s.front() == '"' && s.back() == '"')
         || (s.front() == '\'' && s.back() == '\''))) {
        return s.substr(1, s.size() - 2);
    }
    return s;
}

// ------------------------------------------------------------
// authorize -- shared guard за всички admin endpoints.
// Връща празен optional при success; иначе jsonresponse със статус.
// ------------------------------------------------------------
struct AuthResult {
    bool ok{false};
    int  status{200};
    std::string reason;
};

AuthResult authorize(const crow::request& req)
{
    Garvan::Helper helper;
    std::string token = trimQuotes(helper.getenv("KALPASAN_ADMIN_TOKEN"));

    // Fail-secure: без token → admin off.
    if (token.empty()) {
        return {false, 503, "admin disabled: set KALPASAN_ADMIN_TOKEN in .env"};
    }

    // Loopback-only. Crow's remote_ip_address е "127.0.0.1" или "::1".
    const std::string& ip = req.remote_ip_address;
    if (ip != "127.0.0.1" && ip != "::1") {
        return {false, 403, "admin allowed from loopback only (got " + ip + ")"};
    }

    // Bearer check.
    std::string auth = req.get_header_value("Authorization");
    constexpr std::string_view prefix = "Bearer ";
    if (auth.size() <= prefix.size() || auth.compare(0, prefix.size(), prefix) != 0) {
        return {false, 401, "missing Bearer token"};
    }
    std::string presented = auth.substr(prefix.size());
    if (presented != token) {
        return {false, 401, "invalid token"};
    }
    return {true, 200, {}};
}

jsonresponse authErrorResponse(const AuthResult& a)
{
    auto err = json::Object();
    err["status"] = "error";
    err["error"]  = a.reason;
    return jsonresponse{a.status, err};
}

// ------------------------------------------------------------
// crow::json::rvalue → JsonValue::Object (recursive).
// Приема само object/string/int/double/bool/null/array. Ако user
// подаде nested структура, тя се пренася коректно.
// ------------------------------------------------------------
JsonValue crowToJsonValue(const crow::json::rvalue& r)
{
    switch (r.t()) {
        case crow::json::type::Null:   return JsonValue{nullptr};
        case crow::json::type::True:   return JsonValue{true};
        case crow::json::type::False:  return JsonValue{false};
        case crow::json::type::Number: {
            // Crow не разграничава int/double тук; трактуваме като double
            // и запазваме int64 когато няма fraction.
            double d = r.d();
            if (d == static_cast<int64_t>(d)) return JsonValue{static_cast<int64_t>(d)};
            return JsonValue{d};
        }
        case crow::json::type::String: return JsonValue{std::string(r.s())};
        case crow::json::type::List: {
            auto arr = JsonValue::Array();
            for (const auto& e : r) arr.push_back(crowToJsonValue(e));
            return JsonValue{arr};
        }
        case crow::json::type::Object: {
            auto obj = JsonValue::Object();
            for (const auto& kv : r) {
                obj[std::string(kv.key())] = crowToJsonValue(kv);
            }
            return JsonValue{obj};
        }
        default: return JsonValue{};
    }
}

} // namespace

AdminRoute::AdminRoute(crow::SimpleApp& app)
{
    // -----------------------------------------------------------
    // GET /api/admin/jobs -- изброява регистрираните job класове.
    // -----------------------------------------------------------
    CROW_ROUTE(app, "/api/admin/jobs")
    ([](const crow::request& req) {
        auto a = authorize(req);
        if (!a.ok) return authErrorResponse(a);

        auto out = json::Object();
        auto arr = json::Array();
        for (const auto& n : Garvan::JobRegistry::names()) arr.push_back(n);
        out["registered"] = arr;
        out["count"]      = static_cast<int>(Garvan::JobRegistry::size());
        return jsonresponse{out};
    });

    // -----------------------------------------------------------
    // POST /api/admin/jobs/dispatch
    // body: {"job":"SendTestMail","payload":{"to":"x@y",...}}
    // -----------------------------------------------------------
    CROW_ROUTE(app, "/api/admin/jobs/dispatch").methods("POST"_method)
    ([](const crow::request& req) {
        auto a = authorize(req);
        if (!a.ok) return authErrorResponse(a);

        auto rv = crow::json::load(req.body);
        if (!rv) {
            auto err = json::Object();
            err["status"] = "error";
            err["error"]  = "invalid JSON body";
            return jsonresponse{400, err};
        }
        if (!rv.has("job")) {
            auto err = json::Object();
            err["status"] = "error";
            err["error"]  = "missing 'job' field";
            return jsonresponse{400, err};
        }
        const std::string jobName = std::string(rv["job"].s());
        JsonValue payload = rv.has("payload")
            ? crowToJsonValue(rv["payload"])
            : JsonValue{JsonValue::Object{}};

        try {
            auto job = Garvan::JobRegistry::make(jobName, payload);
            Garvan::JobDispatcher::dispatch(std::move(job));
        } catch (const std::exception& e) {
            auto err = json::Object();
            err["status"] = "error";
            err["error"]  = e.what();
            return jsonresponse{404, err};
        }

        auto ok = json::Object();
        ok["status"] = "dispatched";
        ok["job"]    = jobName;
        return jsonresponse{ok};
    });

    // -----------------------------------------------------------
    // GET /api/admin/events -- изброява регистрираните events
    // (само onezи, за които има EventRegistry bind).
    // -----------------------------------------------------------
    CROW_ROUTE(app, "/api/admin/events")
    ([](const crow::request& req) {
        auto a = authorize(req);
        if (!a.ok) return authErrorResponse(a);

        auto out = json::Object();
        auto arr = json::Array();
        for (const auto& n : Garvan::EventRegistry::names()) arr.push_back(n);
        out["registered"] = arr;
        out["count"]      = static_cast<int>(Garvan::EventRegistry::size());
        return jsonresponse{out};
    });

    // -----------------------------------------------------------
    // POST /api/admin/events/fire
    // body: {"event":"UserRegistered","payload":{...}}
    // -----------------------------------------------------------
    CROW_ROUTE(app, "/api/admin/events/fire").methods("POST"_method)
    ([](const crow::request& req) {
        auto a = authorize(req);
        if (!a.ok) return authErrorResponse(a);

        auto rv = crow::json::load(req.body);
        if (!rv) {
            auto err = json::Object();
            err["status"] = "error";
            err["error"]  = "invalid JSON body";
            return jsonresponse{400, err};
        }
        if (!rv.has("event")) {
            auto err = json::Object();
            err["status"] = "error";
            err["error"]  = "missing 'event' field";
            return jsonresponse{400, err};
        }
        const std::string eventName = std::string(rv["event"].s());
        JsonValue payload = rv.has("payload")
            ? crowToJsonValue(rv["payload"])
            : JsonValue{JsonValue::Object{}};

        std::unique_ptr<Garvan::Event> ev;
        try {
            ev = Garvan::EventRegistry::make(eventName, payload);
        } catch (const std::exception& e) {
            auto err = json::Object();
            err["status"] = "error";
            err["error"]  = e.what();
            return jsonresponse{404, err};
        }

        // listenerCount за отговора трябва да се прочете преди fire
        // (иначе dispatcher-ът може динамично да добави за други
        // сценарии в бъдеще).
        const std::size_t nListeners = Garvan::EventDispatcher::listenerCount(
            std::type_index(typeid(*ev)));

        try {
            Garvan::EventDispatcher::fire(std::move(ev));
        } catch (const std::exception& e) {
            auto err = json::Object();
            err["status"] = "error";
            err["error"]  = e.what();
            return jsonresponse{500, err};
        }

        auto ok = json::Object();
        ok["status"]    = "fired";
        ok["event"]     = eventName;
        ok["listeners"] = static_cast<int>(nListeners);
        return jsonresponse{ok};
    });
}

AdminRoute::~AdminRoute() = default;
