#include "JobServiceProvider.h"

#include "queue/JobRegistry.h"

#include "../jobs/SendTestMail.h"

using namespace AppProviders;

void JobServiceProvider::register_()
{
    // Всеки user job трябва да мине оттук — за pull driver-и
    // (async/database) JobRegistry е единственият начин да се
    // възстанови Job инстанция от persisted payload.
    // Регистрираме с short name (== SendTestMail::jobName()) вместо
    // чрез GARVAN_REGISTER_JOB — макросът stringify-ва целия token
    // ("AppJobs::SendTestMail"), което разминава ключа в registry-то
    // с това, което jobName() връща и което CLI/dispatch-ът очаква.
    Garvan::JobRegistry::bind("SendTestMail", &AppJobs::SendTestMail::fromPayload);
}
