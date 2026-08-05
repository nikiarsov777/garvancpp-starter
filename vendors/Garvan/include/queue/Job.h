#ifndef GARVAN_QUEUE_JOB_H
#define GARVAN_QUEUE_JOB_H

#pragma once

#include <chrono>
#include <memory>
#include <string>

#include "tools/JsonValue.h"

namespace Garvan {

// ---------------------------------------------------------------
// Job -- Laravel-style unit of work.
//
// Синхронна употреба (Фаза A):
//     class SendWelcomeEmail : public Garvan::Job {
//     public:
//         explicit SendWelcomeEmail(int uid) : uid_(uid) {}
//         void handle() override { /* real work */ }
//         std::string jobName() const override { return "SendWelcomeEmail"; }
//     private:
//         int uid_;
//     };
//     JobDispatcher::dispatch(std::make_unique<SendWelcomeEmail>(42));
//
// За persistent queue (Фаза C) allownote-те се `payload()` и
// `fromPayload()` (последното — статичен factory, регистриран в
// JobRegistry). В Фаза A default-ите връщат празни стойности —
// sync driver-ът не ги ползва.
// ---------------------------------------------------------------
class Job {
public:
    virtual ~Job() = default;

    // Тук е бизнес логиката. Хвърли exception при неуспех — sync
    // driver-ът просто ще пропагира; async/DB драйверите (Фаза B/C)
    // ще прилагат retry + backoff политика.
    virtual void handle() = 0;

    // Уникален string идентификатор за класа. Използва се от
    // JobRegistry за възстановяване от payload при persistent
    // queue-ове. Задължителен дори за sync — минимална цена,
    // безценна при debug/log.
    [[nodiscard]] virtual std::string jobName() const = 0;

    // Serialization hooks (Фаза C). За sync driver-а не се викат.
    [[nodiscard]] virtual JsonValue payload() const { return JsonValue::Object(); }

    // Retry policy — приложима когато driver-ът разбира retry.
    [[nodiscard]] virtual int tries() const { return 1; }
    [[nodiscard]] virtual std::chrono::seconds backoff() const { return std::chrono::seconds{0}; }

    // Routing hints. `connection()` избира queue driver:
    //   "sync"     — SyncDriver
    //   "async"    — InMemoryAsyncDriver (Фаза B)
    //   "database" — DatabaseDriver (Фаза C)
    // `queue()` е логическа група (напр. "emails", "reports") —
    // pull driver-ите филтрират по нея.
    [[nodiscard]] virtual std::string connection() const { return "sync"; }
    [[nodiscard]] virtual std::string queue() const { return "default"; }
};

} // namespace Garvan

#endif // GARVAN_QUEUE_JOB_H
