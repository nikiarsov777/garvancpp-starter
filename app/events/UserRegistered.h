#ifndef APP_EVENTS_USERREGISTERED_H
#define APP_EVENTS_USERREGISTERED_H

#pragma once

#include <memory>
#include <string>

#include "events/Event.h"
#include "tools/JsonValue.h"

namespace AppEvents
{

// Fired when a user completes registration. Consumed by:
//   - LogRegistrationListener        (sync, prints audit line)
//   - SendWelcomeEmailListener       (sync, dispatches SendTestMail job)
class UserRegistered : public Garvan::Event
{
public:
    int         userId{0};
    std::string email;
    std::string name;

    UserRegistered() = default;
    UserRegistered(int id, std::string mail, std::string person)
        : userId(id), email(std::move(mail)), name(std::move(person)) {}

    std::string eventName() const override { return "UserRegistered"; }

    Garvan::JsonValue payload() const override;

    // Реконструира event от payload -- използва се от EventRegistry
    // (kalpasan event:fire + admin HTTP endpoint). Всички стойности
    // на wire-a са strings; тук ги привеждаме към реалните типове.
    static std::unique_ptr<Garvan::Event> fromPayload(const Garvan::JsonValue& p);
};

} // namespace AppEvents

#endif
