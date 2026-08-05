#ifndef GARVAN_EVENTS_EVENT_REGISTRY_H
#define GARVAN_EVENTS_EVENT_REGISTRY_H

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Event.h"
#include "tools/JsonValue.h"

namespace Garvan {

// ---------------------------------------------------------------
// EventRegistry -- opt-in factory registry за event class-ове.
//
// EventDispatcher::listen<E, L>() работи изцяло през type_index,
// без string име. Това е type-safe и бързо в hot path, но не
// позволява "fire по име" — сценарий нужен за CLI (kalpasan
// event:fire), admin HTTP endpoint и (в бъдеще) persistent job
// queue за ShouldQueue listener-и.
//
// EventRegistry покрива този use case чрез явна регистрация:
//     EventRegistry::bind("UserRegistered", &UserRegistered::fromPayload);
//
// Регистрацията е opt-in — event-и, които не се вика по име, не
// плащат нищо. `EventDispatcher::listen<>` остава независим.
// ---------------------------------------------------------------
class EventRegistry {
public:
    using Factory = std::function<std::unique_ptr<Event>(const JsonValue&)>;

    static void bind(std::string_view eventName, Factory factory);

    // Хвърля runtime_error ако eventName не е регистриран.
    [[nodiscard]] static std::unique_ptr<Event> make(std::string_view eventName,
                                                       const JsonValue& payload);

    [[nodiscard]] static bool has(std::string_view eventName);
    [[nodiscard]] static std::size_t size();
    [[nodiscard]] static std::vector<std::string> names();
};

} // namespace Garvan

// Помощен макрос за user code:
//     GARVAN_REGISTER_EVENT(UserRegistered);
// Изисква `static std::unique_ptr<Garvan::Event> fromPayload(const Garvan::JsonValue&)`.
#define GARVAN_REGISTER_EVENT(ClassName) \
    ::Garvan::EventRegistry::bind(#ClassName, &ClassName::fromPayload)

#endif // GARVAN_EVENTS_EVENT_REGISTRY_H
