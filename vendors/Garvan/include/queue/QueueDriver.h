#ifndef GARVAN_QUEUE_QUEUE_DRIVER_H
#define GARVAN_QUEUE_QUEUE_DRIVER_H

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "Job.h"
#include "tools/JsonValue.h"

namespace Garvan {

// ---------------------------------------------------------------
// QueuedJob -- envelope, който pull driver-ите връщат при `pop()`.
//
// За sync driver-а не се използва (той не поддържа pop).
// `id` е driver-specific:
//   - InMemoryAsyncDriver: монотонно нарастващ counter в паметта.
//   - DatabaseDriver:      реален PK от `jobs` таблицата.
// ---------------------------------------------------------------
struct QueuedJob {
    std::int64_t id{0};
    std::string  jobName;
    JsonValue    payload;
    int          attempts{0};
    std::string  queue{"default"};
};

// ---------------------------------------------------------------
// QueueDriver -- абстракция над "къде отиват jobs".
//
// Push-only drivers (sync) имплементират само `push`. Pull drivers
// (async / database) допълнително имплементират `pop` +
// `markCompleted`/`markFailed`. Default-ните имплементации на pop*
// връщат nullopt / no-op, за да останат sync driver-ите тривиални.
// ---------------------------------------------------------------
class QueueDriver {
public:
    virtual ~QueueDriver() = default;

    // Приема ownership на Job-а. Sync driver-ът го изпълнява
    // веднага; pull driver-ите го сериализират в опашка.
    virtual void push(std::unique_ptr<Job> job) = 0;

    // Pull API — по default no-op. Worker loop-ът (Фаза C) вика
    // pop в цикъл; drivers, които не поддържат pull, връщат
    // празно и worker-ът заспива.
    virtual std::optional<QueuedJob> pop([[maybe_unused]] std::string_view queue) {
        return std::nullopt;
    }

    virtual void markCompleted([[maybe_unused]] const QueuedJob& q) {}
    virtual void markFailed([[maybe_unused]] const QueuedJob& q,
                            [[maybe_unused]] std::string_view error) {}

    // Уникално име ("sync", "async", "database", ...). Използва се
    // от JobDispatcher за routing на база `job->connection()`.
    [[nodiscard]] virtual std::string name() const = 0;
};

} // namespace Garvan

#endif // GARVAN_QUEUE_QUEUE_DRIVER_H
