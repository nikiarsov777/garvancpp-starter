#include "UserRegistered.h"

#include <cstdlib>

using namespace AppEvents;

Garvan::JsonValue UserRegistered::payload() const
{
    auto p = Garvan::JsonValue::Object();
    p["user_id"] = userId;
    p["email"]   = email;
    p["name"]    = name;
    return p;
}

std::unique_ptr<Garvan::Event> UserRegistered::fromPayload(const Garvan::JsonValue& p)
{
    // JsonValue::operator[] е read-only на const обект. Helper за
    // безопасно extract-ване на string стойност (default: празно).
    auto asStr = [&](const char* key) -> std::string {
        try {
            return p[key].isString() ? p[key].asString() : std::string{};
        } catch (...) { return {}; }
    };
    const std::string idStr = asStr("user_id");
    const int id = idStr.empty() ? 0 : std::atoi(idStr.c_str());
    return std::make_unique<UserRegistered>(id, asStr("email"), asStr("name"));
}
