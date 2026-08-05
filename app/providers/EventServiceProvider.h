#ifndef APP_PROVIDERS_EVENT_SERVICE_PROVIDER_H
#define APP_PROVIDERS_EVENT_SERVICE_PROVIDER_H

#pragma once

#include "app/ServiceProvider.h"

namespace AppProviders
{

// ---------------------------------------------------------------
// EventServiceProvider -- обвързва event → listener.
//
//   EventDispatcher::listen<UserRegistered, SendWelcomeEmailListener>();
//
// ListenerT трябва да е default-constructible; EventDispatcher
// създава нова инстанция на всеки fire, така че state в listener-а
// не се пази между извиквания (по подразбиране).
// ---------------------------------------------------------------
class EventServiceProvider : public Garvan::ServiceProvider
{
public:
    void register_() override;
};

} // namespace AppProviders

#endif
