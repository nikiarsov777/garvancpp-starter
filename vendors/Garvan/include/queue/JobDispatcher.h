#ifndef GARVAN_QUEUE_JOB_DISPATCHER_H
#define GARVAN_QUEUE_JOB_DISPATCHER_H

#pragma once

#include <memory>
#include <string_view>

#include "Job.h"
#include "QueueDriver.h"

namespace Garvan {

// ---------------------------------------------------------------
// JobDispatcher -- fascade за push към различни queue driver-и.
//
// Startup: `bind("sync", std::make_unique<SyncDriver>())` (обикновено
// в `AppServiceProvider::register_()`). Runtime:
//     JobDispatcher::dispatch(std::make_unique<SendWelcomeEmail>(uid));
//
// Routing: job-ът декларира `connection()` (default "sync") и
// диспачърът избира съответно регистрирания driver. Ако липсва
// driver за тази connection — грешка при runtime (fail-fast).
//
// `dispatchSync` е "force-sync" escape hatch — независимо какво
// казва `job->connection()`, пуска SyncDriver-a. Полезно за тестове
// и за bootstrap логика, която не трябва да минава през queue.
// ---------------------------------------------------------------
class JobDispatcher {
public:
    static void bind(std::string_view connection, std::unique_ptr<QueueDriver> driver);

    // Обичайният dispatch: рутира според job->connection().
    static void dispatch(std::unique_ptr<Job> job);

    // Форсиран sync execution — игнорира connection().
    static void dispatchSync(std::unique_ptr<Job> job);

    // За тестове и за EventDispatcher::fire (queued listener path).
    [[nodiscard]] static QueueDriver* driver(std::string_view connection);

    static void clear();
};

} // namespace Garvan

#endif // GARVAN_QUEUE_JOB_DISPATCHER_H
