#ifndef GARVAN_EVENTS_EVENT_DISPATCHER_H
#define GARVAN_EVENTS_EVENT_DISPATCHER_H

#pragma once

#include <functional>
#include <memory>
#include <string_view>
#include <type_traits>
#include <typeindex>

#include "Event.h"
#include "Listener.h"

namespace Garvan {

// ---------------------------------------------------------------
// EventDispatcher -- Laravel-style event/listener bus.
//
// Регистрация (обикновено в `EventServiceProvider::register_()`):
//     EventDispatcher::listen<UserRegistered, SendWelcomeEmail>();
//
// Firing:
//     EventDispatcher::fire(std::make_unique<UserRegistered>(uid, email));
//
// Routing:
//   * Sync listener → извиква се веднага в calling thread.
//   * ShouldQueue listener → wrap-ва се в internal ListenerJob и
//     минава през JobDispatcher (connection/queue се четат от
//     ListenerT-a при instantiate).
//
// Type safety: event се съхранява по `type_index(EventT)` — при
// fire lookup-ът е точен за конкретния derived тип. Ако fire-неш
// derived event, но си listen-нал само за базовия — няма match.
// Това е нарочно (Laravel-style semantics с точен class match).
// ---------------------------------------------------------------
class EventDispatcher {
public:
    // Регистрира listener клас за конкретен event тип. ListenerT
    // трябва да е default-constructible (или sync-only ListenerBase
    // subclass със същия конструктор).
    template <typename EventT, typename ListenerT>
    static void listen()
    {
        static_assert(std::is_base_of_v<Event, EventT>,
                      "EventT must derive from Garvan::Event");
        static_assert(std::is_base_of_v<Listener<EventT>, ListenerT>,
                      "ListenerT must derive from Garvan::Listener<EventT>");
        static_assert(std::is_default_constructible_v<ListenerT>,
                      "ListenerT must be default-constructible "
                      "(reconstructed on every fire; state via ctor е забранено)");

        // Factory: създава fresh listener на всеки fire. Държим
        // `is_queued` bit-а тук за да избегнем `dynamic_cast` при
        // fire (по-бързо в hot path).
        constexpr bool queued = std::is_base_of_v<ShouldQueue, ListenerT>;

        Factory f = []() -> std::unique_ptr<ListenerBase> {
            return std::make_unique<ListenerT>();
        };

        registerFactory(std::type_index(typeid(EventT)),
                        Entry{std::move(f), queued});
    }

    // Fire event: всички регистрирани listener-и за точния тип
    // получават копие/референция. Ownership на event-a се държи
    // от dispatcher-а за целия fire (нужно за queued path — job-ът
    // взима shared ownership).
    static void fire(std::unique_ptr<Event> event);

    // Testing / kalpasan helpers.
    [[nodiscard]] static std::size_t listenerCount(std::type_index eventType);
    static void clear();

    // Публично само защото .cpp-то (в anon namespace) държи
    // storage-а. Не пипай директно — използвай `listen<>()`.
    using Factory = std::function<std::unique_ptr<ListenerBase>()>;
    struct Entry {
        Factory factory;
        bool    queued;
    };

private:
    static void registerFactory(std::type_index eventType, Entry entry);
};

} // namespace Garvan

#endif // GARVAN_EVENTS_EVENT_DISPATCHER_H
