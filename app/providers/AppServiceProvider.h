#ifndef APP_PROVIDERS_APP_SERVICE_PROVIDER_H
#define APP_PROVIDERS_APP_SERVICE_PROVIDER_H

#pragma once

#include "app/ServiceProvider.h"

namespace AppProviders
{

// ---------------------------------------------------------------
// AppServiceProvider -- първи в реда, wire-ва инфраструктурата.
//
// Тук се регистрират queue driver-ите в JobDispatcher (sync в Phase
// A; async + database ще се добавят в следващи фази). Всички
// останали providers могат да разчитат, че `sync` driver-ът е
// наличен.
// ---------------------------------------------------------------
class AppServiceProvider : public Garvan::ServiceProvider
{
public:
    void register_() override;
    void boot() override;
};

} // namespace AppProviders

#endif
