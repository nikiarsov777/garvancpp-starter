#ifndef GARVAN_QUEUE_JOB_REGISTRY_H
#define GARVAN_QUEUE_JOB_REGISTRY_H

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Job.h"
#include "tools/JsonValue.h"

namespace Garvan {

// ---------------------------------------------------------------
// JobRegistry -- resolver `jobName -> factory(payload) -> Job`.
//
// Регистрира се в `JobServiceProvider::register_()` в starter-а:
//     JobRegistry::bind("SendWelcomeEmail", &SendWelcomeEmail::fromPayload);
//
// Използва се от pull driver-ите (async/database) при dequeue за
// възстановяване на Job инстанция от persisted payload. SyncDriver
// НЕ преминава през registry — той държи оригиналната инстанция.
//
// Registry-то е process-global (Meyers singleton), защитено с
// mutex защото се обновява от главния thread (bootAll) и се чете
// от worker thread-овете.
// ---------------------------------------------------------------
class JobRegistry {
public:
    using Factory = std::function<std::unique_ptr<Job>(const JsonValue&)>;

    static void bind(std::string_view jobName, Factory factory);

    // Хвърля runtime_error ако jobName не е регистриран.
    [[nodiscard]] static std::unique_ptr<Job> make(std::string_view jobName,
                                                    const JsonValue& payload);

    // За тестове / kalpasan config:show.
    [[nodiscard]] static bool has(std::string_view jobName);
    [[nodiscard]] static std::size_t size();

    // Enumeration -- използва се от admin endpoint /api/admin/jobs
    // и kalpasan `job:list`. Редът не е гарантиран.
    [[nodiscard]] static std::vector<std::string> names();
};

} // namespace Garvan

// Помощен макрос: свежда регистрацията до един ред при типични
// job-ове с публичен `static std::unique_ptr<Garvan::Job>
// fromPayload(const Garvan::JsonValue&)`.
//
//     GARVAN_REGISTER_JOB(SendWelcomeEmail);
#define GARVAN_REGISTER_JOB(ClassName) \
    ::Garvan::JobRegistry::bind(#ClassName, &ClassName::fromPayload)

#endif // GARVAN_QUEUE_JOB_REGISTRY_H
