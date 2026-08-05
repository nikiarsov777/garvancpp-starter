#ifndef GARVAN_APP_SERVICE_PROVIDER_H
#define GARVAN_APP_SERVICE_PROVIDER_H

#pragma once

namespace Garvan {

// ---------------------------------------------------------------
// ServiceProvider -- Laravel-style boot hook.
//
// AppKernel::bootAll() изпълнява в две фази:
//   1. За всеки provider — register_():
//        Тук вършим "bindings" — например регистрация на queue
//        driver-и в JobDispatcher, connecting на JobRegistry
//        factory-та, EventDispatcher::listen<> обвързвания.
//        Може да разчиташ, че container / logger са налични, но
//        НЕ и че други providers са завършили register_().
//   2. За всеки provider — boot():
//        Тук всички bindings са готови. Тук се пускат background
//        threads (Фаза B), лениви singletons, warm-up cache, etc.
//
// Rule of thumb: страничен ефект → boot(); чиста регистрация →
// register_().
//
// Името е `register_()` защото `register` е reserved keyword в
// C++. Иначе API 1:1 с Laravel.
// ---------------------------------------------------------------
class ServiceProvider {
public:
    virtual ~ServiceProvider() = default;

    virtual void register_() {}
    virtual void boot() {}
};

} // namespace Garvan

#endif // GARVAN_APP_SERVICE_PROVIDER_H
