#include "SendWelcomeEmailListener.h"

#include <memory>
#include <string>

#include "queue/JobDispatcher.h"
#include "../jobs/SendTestMail.h"

using namespace AppListeners;

void SendWelcomeEmailListener::handle(const AppEvents::UserRegistered& event)
{
    // Строим welcome mail на база event data и го dispatch-ваме
    // през активния queue driver. В Phase A това е sync driver-a,
    // така че `SendTestMail::handle` ще се изпълни веднага в този
    // же callstack. В Phase B/C ще отиде към async / database.
    const std::string subject = "Welcome to BgChess Zone, " + event.name + "!";
    const std::string body    = "Hi " + event.name + ",\n\n"
                                "Thanks for signing up. Your user id is #"
                                + std::to_string(event.userId) + ".\n"
                                "-- BgChess Zone";

    Garvan::JobDispatcher::dispatch(
        std::make_unique<AppJobs::SendTestMail>(event.email, subject, body));
}
