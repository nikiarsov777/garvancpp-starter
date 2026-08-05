#ifndef GARVAN_QUEUE_SYNC_DRIVER_H
#define GARVAN_QUEUE_SYNC_DRIVER_H

#pragma once

#include "../QueueDriver.h"

namespace Garvan {

// ---------------------------------------------------------------
// SyncDriver -- executes `job->handle()` in the calling thread
// immediately. Exceptions пропагират — caller-ът решава как да ги
// третира. Никаква serialization, никаква persistence, никакво
// retry management (`tries()`/`backoff()` се игнорират).
//
// Първата и най-проста имплементация — за development, тестове и
// за случаи, в които "queue" е просто индирекция за да може по-
// късно да се смени driver-ът без промени в call sites.
// ---------------------------------------------------------------
class SyncDriver final : public QueueDriver {
public:
    void push(std::unique_ptr<Job> job) override;
    [[nodiscard]] std::string name() const override { return "sync"; }
};

} // namespace Garvan

#endif // GARVAN_QUEUE_SYNC_DRIVER_H
