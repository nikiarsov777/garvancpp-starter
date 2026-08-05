#include "LogRegistrationListener.h"

#include <cstdio>

using namespace AppListeners;

void LogRegistrationListener::handle(const AppEvents::UserRegistered& event)
{
    std::fprintf(stdout,
        "[LogRegistrationListener] user #%d %s <%s> registered\n",
        event.userId, event.name.c_str(), event.email.c_str());
    std::fflush(stdout);
}
