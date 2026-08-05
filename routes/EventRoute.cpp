#include "EventRoute.h"

#include <memory>
#include <string>
#include <typeindex>

#include "events/EventDispatcher.h"
#include "../app/events/UserRegistered.h"

using namespace Routes;

namespace {

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

EventRoute::EventRoute(crow::SimpleApp& app)
{
    // ---------------------------------------------------------------
    // GET /api/events/user-registered?email=X[&name=Y][&id=N]
    //
    // Изстрелва AppEvents::UserRegistered. EventDispatcher намира
    // всички регистрирани listener-и за този type_index и ги
    // изпълнява по ред на регистрация:
    //   1. LogRegistrationListener   (sync log)
    //   2. SendWelcomeEmailListener  (sync -> dispatch SendTestMail)
    // ---------------------------------------------------------------
    CROW_ROUTE(app, "/api/events/user-registered")
    ([](const crow::request& req) {
        // Всички query параметри са optional. Дефолтите създават
        // валиден UserRegistered event, така че smoke test (curl без
        // args) винаги минава през пълния listener chain.
        std::string email = req.url_params.get("email") ? req.url_params.get("email") : "demo@bgchess.zone";
        std::string name  = req.url_params.get("name")  ? req.url_params.get("name")  : "Anonymous";
        int         id    = req.url_params.get("id")    ? std::atoi(req.url_params.get("id")) : 0;

        // listenerCount се чете преди fire — гарантира консистентен
        // отговор дори ако fire добавя dynamic listener-и в бъдеще.
        const std::size_t nListeners = Garvan::EventDispatcher::listenerCount(
            std::type_index(typeid(AppEvents::UserRegistered)));

        try {
            Garvan::EventDispatcher::fire(
                std::make_unique<AppEvents::UserRegistered>(id, email, name));
        } catch (const std::exception& e) {
            json err = json::Object();
            err["status"] = "error";
            err["error"]  = e.what();
            return jsonresponse{err};
        }

        json ok = json::Object();
        ok["status"]    = "fired";
        ok["event"]     = "UserRegistered";
        ok["listeners"] = static_cast<int>(nListeners);
        ok["payload"]   = AppEvents::UserRegistered(id, email, name).payload();
        return jsonresponse{ok};
    });

    // ---------------------------------------------------------------
    // GET /api/events/status
    // Debug: колко listener-а има за известните ни event-и.
    // ---------------------------------------------------------------
    CROW_ROUTE(app, "/api/events/status")
    ([]() {
        json out = json::Object();
        auto known = json::Object();
        known["UserRegistered"] = static_cast<int>(
            Garvan::EventDispatcher::listenerCount(
                std::type_index(typeid(AppEvents::UserRegistered))));
        out["listeners"] = known;
        return jsonresponse{out};
    });
}

EventRoute::~EventRoute() = default;
