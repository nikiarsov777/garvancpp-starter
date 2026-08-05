#ifndef APP_JOBS_SENDTESTMAIL_H
#define APP_JOBS_SENDTESTMAIL_H

#pragma once

#include <memory>
#include <string>

#include "queue/Job.h"
#include "tools/JsonValue.h"

namespace AppJobs
{

// SendTestMail -- demo job that simulates dispatching an email
// through the MAIL_* settings in .env. There is no SMTP client
// linked into the framework yet, so `handle()` only reads the
// MAIL_DRIVER/HOST/PORT/USERNAME/FROM_ADDRESS values via
// Garvan::Helper::getenv and prints a structured line. Swap in a
// real SMTP client (e.g. libcurl) later without changing this API.
class SendTestMail : public Garvan::Job
{
public:
    SendTestMail() = default;
    SendTestMail(std::string to, std::string subject, std::string body)
        : to_(std::move(to)), subject_(std::move(subject)), body_(std::move(body)) {}

    ~SendTestMail() override = default;

    void handle() override;

    std::string jobName() const override { return "SendTestMail"; }

    // Persistence hooks — ready for the future database driver.
    Garvan::JsonValue payload() const override;
    static std::unique_ptr<Garvan::Job> fromPayload(const Garvan::JsonValue& p);

    // Route via SyncDriver in Phase A. Switch to "async"/"database"
    // once those drivers are bound in AppServiceProvider.
    std::string connection() const override { return "sync"; }

private:
    std::string to_;
    std::string subject_;
    std::string body_;
};

} // namespace AppJobs

#endif
