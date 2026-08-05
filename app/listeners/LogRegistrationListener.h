#ifndef APP_LISTENERS_LOGREGISTRATIONLISTENER_H
#define APP_LISTENERS_LOGREGISTRATIONLISTENER_H

#pragma once

#include "events/Listener.h"
#include "../events/UserRegistered.h"

namespace AppListeners
{

// Pure sync listener — writes an audit line to stdout. No side
// effects beyond logging. Demonstrates the simplest possible
// listener path: `EventDispatcher::fire` -> `handle` inline in the
// caller's thread.
class LogRegistrationListener
    : public Garvan::Listener<AppEvents::UserRegistered>
{
public:
    LogRegistrationListener() = default;
    ~LogRegistrationListener() override = default;

    void handle(const AppEvents::UserRegistered& event) override;
};

} // namespace AppListeners

#endif
