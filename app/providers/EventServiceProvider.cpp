#include "EventServiceProvider.h"

#include "events/EventDispatcher.h"
#include "events/EventRegistry.h"

#include "../events/UserRegistered.h"
#include "../listeners/LogRegistrationListener.h"
#include "../listeners/SendWelcomeEmailListener.h"

using namespace AppProviders;

void EventServiceProvider::register_()
{
    // 1) Listener wiring — ред на регистрация = ред на изпълнение
    //    при fire. Log-ването върви първо, за audit line преди
    //    side-effect-ната верига през job dispatcher-a.
    Garvan::EventDispatcher::listen<
        AppEvents::UserRegistered,
        AppListeners::LogRegistrationListener>();

    Garvan::EventDispatcher::listen<
        AppEvents::UserRegistered,
        AppListeners::SendWelcomeEmailListener>();

    // 2) EventRegistry bind — opt-in регистрация по име, позволява
    //    `kalpasan event:fire UserRegistered ...` и admin endpoint
    //    POST /api/admin/events/fire да реконструират event от
    //    payload. Без този bind event-ът остава fire-в само от C++
    //    код (типизиран path през `EventDispatcher::fire`).
    GARVAN_REGISTER_EVENT(AppEvents::UserRegistered);
}
