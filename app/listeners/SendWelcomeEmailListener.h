#ifndef APP_LISTENERS_SENDWELCOMEEMAILLISTENER_H
#define APP_LISTENERS_SENDWELCOMEEMAILLISTENER_H

#pragma once

#include "events/Listener.h"
#include "../events/UserRegistered.h"

namespace AppListeners
{

// Sync listener that dispatches a SendTestMail job. Two-hop
// pattern: EventDispatcher::fire(UserRegistered) -> handle() ->
// JobDispatcher::dispatch(SendTestMail). Demonstrates the
// "listener triggers side-effect job" wiring used everywhere in
// Laravel.
class SendWelcomeEmailListener
    : public Garvan::Listener<AppEvents::UserRegistered>
{
public:
    SendWelcomeEmailListener() = default;
    ~SendWelcomeEmailListener() override = default;

    void handle(const AppEvents::UserRegistered& event) override;
};

} // namespace AppListeners

#endif
