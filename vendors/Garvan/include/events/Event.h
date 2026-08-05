#ifndef GARVAN_EVENTS_EVENT_H
#define GARVAN_EVENTS_EVENT_H

#pragma once

#include <string>

#include "tools/JsonValue.h"

namespace Garvan {

// ---------------------------------------------------------------
// Event -- POD-like domain notification.
//
// User dispatch pattern:
//     class UserRegistered : public Garvan::Event {
//     public:
//         int userId;
//         std::string email;
//
//         std::string eventName() const override { return "UserRegistered"; }
//         Garvan::JsonValue payload() const override {
//             auto p = Garvan::JsonValue::Object();
//             p["user_id"] = userId;
//             p["email"]   = email;
//             return p;
//         }
//     };
//     EventDispatcher::fire(std::make_unique<UserRegistered>(uid, email));
//
// `payload()` е нужен само за queued listener-и (ShouldQueue) —
// сериализира се в job envelope. За чисто sync listener-и — може
// да върне празен обект.
// ---------------------------------------------------------------
class Event {
public:
    virtual ~Event() = default;

    // Уникално, човешко-четимо име (обикновено class name).
    // Използва се при dispatch на ShouldQueue listener-и, за да
    // може ListenerJob да го запише в payload-а и после да
    // възстанови точния event тип.
    [[nodiscard]] virtual std::string eventName() const = 0;

    // Serialization за queued listener path. Sync path НЕ вика
    // payload() — просто предава референция към живия event.
    [[nodiscard]] virtual JsonValue payload() const { return JsonValue::Object(); }
};

} // namespace Garvan

#endif // GARVAN_EVENTS_EVENT_H
