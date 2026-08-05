#ifndef GARVAN_EVENTS_LISTENER_JOB_H
#define GARVAN_EVENTS_LISTENER_JOB_H

#pragma once

#include <memory>
#include <string>
#include <utility>

#include "../queue/Job.h"
#include "Event.h"
#include "Listener.h"

namespace Garvan {

// ---------------------------------------------------------------
// ListenerJob -- internal wrapper, който позволява на
// EventDispatcher да маршрутизира ShouldQueue listener-и през
// JobDispatcher.
//
// Фаза A (sync): държим живата listener + event инстанция и просто
// извикваме `listener->dispatch(*event)` в `handle()`. Никаква
// serialization не се прави — SyncDriver-ът така или иначе не
// сериализира.
//
// Фаза C (persistent): payload()/fromPayload() ще носят
// `{listener_name, event_name, event_payload}` и ще се минава през
// JobRegistry за възстановяване. Засега тези хукове връщат
// достатъчно за debug.
// ---------------------------------------------------------------
class ListenerJob final : public Job {
public:
    ListenerJob(std::unique_ptr<ListenerBase> listener,
                std::shared_ptr<Event>       event,
                std::string                  connection,
                std::string                  queue,
                std::string                  listenerName)
        : listener_(std::move(listener))
        , event_(std::move(event))
        , connection_(std::move(connection))
        , queue_(std::move(queue))
        , listenerName_(std::move(listenerName))
    {}

    void handle() override {
        listener_->dispatch(*event_);
    }

    [[nodiscard]] std::string jobName() const override {
        // Prefix за да не може user job да resegue с това име.
        return "__ListenerJob:" + listenerName_;
    }

    [[nodiscard]] std::string connection() const override { return connection_; }
    [[nodiscard]] std::string queue()      const override { return queue_; }

    [[nodiscard]] JsonValue payload() const override {
        auto p = JsonValue::Object();
        p["listener"]   = listenerName_;
        p["event_name"] = event_ ? event_->eventName() : std::string{};
        if (event_) p["event_payload"] = event_->payload();
        return p;
    }

private:
    std::unique_ptr<ListenerBase> listener_;
    std::shared_ptr<Event>        event_;
    std::string                   connection_;
    std::string                   queue_;
    std::string                   listenerName_;
};

} // namespace Garvan

#endif // GARVAN_EVENTS_LISTENER_JOB_H
