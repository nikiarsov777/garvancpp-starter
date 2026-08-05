#ifndef GARVAN_EVENTS_LISTENER_H
#define GARVAN_EVENTS_LISTENER_H

#pragma once

#include "Event.h"

namespace Garvan {

// ---------------------------------------------------------------
// Listener<EventT> -- type-safe listener base.
//
//     class SendWelcomeEmail : public Garvan::Listener<UserRegistered> {
//     public:
//         void handle(const UserRegistered& e) override {
//             // real work
//         }
//     };
//
// `EventT` трябва да наследява `Garvan::Event`. Наличието на
// нетипизиран `handle(const Event&)` в базата ни позволява
// EventDispatcher-ът да държи `unique_ptr<ListenerBase>` без да
// знае конкретния event тип.
// ---------------------------------------------------------------
class ListenerBase {
public:
    virtual ~ListenerBase() = default;

    // Извиква се от EventDispatcher::fire със слейпаза върху event-a.
    // Реализациите в `Listener<EventT>` правят static_cast към
    // конкретния тип и делегират към typed handle().
    virtual void dispatch(const Event& event) = 0;
};

template <typename EventT>
class Listener : public ListenerBase {
public:
    // User-facing typed API.
    virtual void handle(const EventT& event) = 0;

    void dispatch(const Event& event) override {
        // Безопасно: EventDispatcher::listen<EventT, ...> гарантира,
        // че този listener няма да получи друг тип event.
        handle(static_cast<const EventT&>(event));
    }
};

// ---------------------------------------------------------------
// ShouldQueue -- marker interface. Listener, който го наследява,
// казва на EventDispatcher: "не ме викай синхронно, изпрати ме
// през JobDispatcher".
//
//     class ShipReceipt : public Garvan::Listener<OrderPaid>,
//                         public Garvan::ShouldQueue {
//     public:
//         void handle(const OrderPaid& e) override { /* ... */ }
//         std::string connection() const override { return "database"; }
//         std::string queue()      const override { return "emails"; }
//     };
//
// `connection()` / `queue()` контролират routing-а на generated
// wrapper Job-a. Default: connection="sync" (може да се override-не
// от derived клас; ShouldQueue не форсира конкретен driver).
// ---------------------------------------------------------------
class ShouldQueue {
public:
    virtual ~ShouldQueue() = default;

    [[nodiscard]] virtual std::string connection() const { return "sync"; }
    [[nodiscard]] virtual std::string queue() const { return "default"; }
};

} // namespace Garvan

#endif // GARVAN_EVENTS_LISTENER_H
