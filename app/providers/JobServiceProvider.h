#ifndef APP_PROVIDERS_JOB_SERVICE_PROVIDER_H
#define APP_PROVIDERS_JOB_SERVICE_PROVIDER_H

#pragma once

#include "app/ServiceProvider.h"

namespace AppProviders
{

// ---------------------------------------------------------------
// JobServiceProvider -- регистрира user job класове в JobRegistry.
//
// Регистрацията е нужна за pull drivers (async/database) — те
// възстановяват Job инстанции от persisted payload по class name.
// За sync driver-a е технически излишна, но добра практика: всеки
// user job трябва да мине оттук, за да не се пропуска регистрация
// при бъдещ switch към database.
//
// Пример:
//   #include "../jobs/SendWelcomeEmail.h"
//   void JobServiceProvider::register_() {
//       GARVAN_REGISTER_JOB(AppJobs::SendWelcomeEmail);
//   }
// ---------------------------------------------------------------
class JobServiceProvider : public Garvan::ServiceProvider
{
public:
    void register_() override;
};

} // namespace AppProviders

#endif
