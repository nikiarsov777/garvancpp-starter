#ifndef GARVAN_APP_APP_KERNEL_H
#define GARVAN_APP_APP_KERNEL_H

#pragma once

#include <memory>
#include <vector>

#include "ServiceProvider.h"

namespace Garvan {

// ---------------------------------------------------------------
// AppKernel -- singleton, който държи регистрираните
// ServiceProvider-и и оркестрира boot / shutdown жизнения цикъл.
//
// Употреба в starter/main.cpp:
//     auto& k = Garvan::AppKernel::instance();
//     k.addProvider(std::make_unique<AppServiceProvider>());
//     k.addProvider(std::make_unique<JobServiceProvider>());
//     k.addProvider(std::make_unique<EventServiceProvider>());
//     k.bootAll();
//
//     ... стартираш Crow app-a ...
//
//     k.shutdown();  // при graceful exit
//
// Ordering:
//   - Providers се пускат в реда, в който са добавени.
//   - register_() на всички → boot() на всички (mirror Laravel).
//   - shutdown() върви в reverse order — по-скорошно добавените
//     providers се спират първи (LIFO), за да могат по-долните
//     да разчитат на инфраструктура при tear-down.
// ---------------------------------------------------------------
class AppKernel {
public:
    static AppKernel& instance();

    // Non-copyable, non-movable.
    AppKernel(const AppKernel&)            = delete;
    AppKernel& operator=(const AppKernel&) = delete;
    AppKernel(AppKernel&&)                 = delete;
    AppKernel& operator=(AppKernel&&)      = delete;

    // Приема ownership. Извикайте преди `bootAll()`.
    void addProvider(std::unique_ptr<ServiceProvider> p);

    // Първо `register_()` на всеки provider (в reg order), после
    // `boot()` на всеки. Идемпотентно — повторно повикване е no-op
    // (защита срещу двойни boot-ове при hot-reload сценарии).
    void bootAll();

    // Reverse-order teardown. Извиква `shutdown()` hook (ако
    // ServiceProvider го override-не в бъдеще) и после унищожава
    // providers-ите. Безопасно е да се вика без bootAll().
    void shutdown();

    // За тестове.
    [[nodiscard]] std::size_t providerCount() const;
    [[nodiscard]] bool        booted() const;

private:
    AppKernel() = default;
    ~AppKernel() = default;

    std::vector<std::unique_ptr<ServiceProvider>> providers_;
    bool booted_{false};
};

} // namespace Garvan

#endif // GARVAN_APP_APP_KERNEL_H
