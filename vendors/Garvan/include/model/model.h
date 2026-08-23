#ifndef GARVAN_MODEL_H
#define GARVAN_MODEL_H

#include "../orm/omodel.h"
#include "../orm/builder.h"
#include "../db/DbClient.h"
#include <concepts>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <iostream>

namespace Garvan
{

class Model;
template <typename T> class TypedQuery;   // orm/typed_query.h

// ---------------------------------------------------------------
// ModelType concept — контракт за типовете, използвани със
// статичните шаблони `Model::where<T>()`, `find<T>()`, `get<T>()`.
// Изисква публично наследяване от `Garvan::Model` и default
// конструктор. Грешките при неправилен `T` вече идват като кратко
// concept failure вместо стотици редове template instantiation
// шум.
// ---------------------------------------------------------------
template <typename T>
concept ModelType = std::derived_from<T, Model>
                 && std::is_default_constructible_v<T>;

class Model : public ORM::OModel
{
public:
    explicit Model(std::string client = "PSQL");
    virtual ~Model(); // Виртуален за правилно изтриване на наследници

    void init();

    // --- СТАТИЧНИ МЕТОДИ (ВРЪЩАТ УМЕН УКАЗАТЕЛ) ---
    template <ModelType T>
    static std::unique_ptr<T> fastWhere(std::string field, std::string value) {
        auto instance = std::make_unique<T>();
        instance->isPointer = true;
        instance->init();
        instance->where(field, value);
        return instance;
    }

    template <ModelType T>
    static std::unique_ptr<T> fastWhere(std::string field, std::string op, std::string value) {
        auto instance = std::make_unique<T>();
        instance->isPointer = true;
        instance->init();
        instance->where(field, op, value);
        return instance;
    }

    // ------------------------------------------------------------
    // Static factory: `where<T>(...)` връща суров `T*` за chaining
    // (BC — starter go ползва точно така). Ownership вече НЕ се
    // предава на терминалния метод (`delete this` е премахнат);
    // вместо това инстанцията се държи в per-thread scratchpad
    // (`registerScratch`) и се освобождава при следващия `where<T>`
    // на същия thread или при явен `Model::flushScratch()`.
    //
    // Тази стратегия държи стария call site 100% работещ, без да
    // тече паметта и без undefined behaviour при stack-allocated
    // модели (виж коментара в `Model::find<T>` за старото поведение).
    // ------------------------------------------------------------
    template <ModelType T>
    static T* where(std::string field, std::string value) {
        auto owned = std::make_unique<T>();
        T* raw = owned.get();
        raw->isPointer = true;    // suppress old `delete this` path (paranoia)
        raw->init();
        raw->where(field, value);
        registerScratch(std::move(owned));
        return raw;
    }

    template <ModelType T>
    static T* where(std::string field, std::string op, std::string value) {
        auto owned = std::make_unique<T>();
        T* raw = owned.get();
        raw->isPointer = true;
        raw->init();
        raw->where(field, op, value);
        registerScratch(std::move(owned));
        return raw;
    }

    // Ръчно освобождаване на per-thread scratchpad-а (например
    // между заявки в дългоживущ worker). Извиква се и автоматично
    // в началото на всеки `where<T>()` — активно държи само последно
    // регистрираните модели.
    static void flushScratch() noexcept;

private:
    // Регистрира модел в per-thread scratchpad. Дефиниран в
    // `model.cpp` за да остане списъкът локален за библиотеката.
    static void registerScratch(std::unique_ptr<Model> instance);

public:

    template <ModelType T>
    static json find(int id) {
        T instance;
        // The terminal builder methods (find/get/first/...) call
        // `delete this` whenever `isPointer` is false. Since this
        // instance lives on the stack, that would be undefined
        // behaviour (deleting a non-heap object). Marking it as a
        // "pointer-owned" object suppresses the self-delete; the
        // stack destructor still runs at scope exit.
        instance.isPointer = true;
        instance.init();
        return instance.find(id);
    }

    template <ModelType T>
    static json get() {
        T instance;
        instance.isPointer = true;   // see comment in find<T>() above
        instance.init();
        return instance.get();
    }

    // ============================================================
    // Typed pipeline (see `GARVAN.md` -- "Какво искам" section).
    //
    //   User user = User::query<User>()->where("email",e)->first().value();
    //   User user = User::findAs<User>(id);          // throws if missing
    //   auto  opt = User::tryFindAs<User>(id);       // std::optional<User>
    //
    // The typed wrapper (`Garvan::TypedQuery<T>`) is defined in
    // `orm/typed_query.h`, which is included below to keep the entry
    // points on `Model` without forcing consumers to touch another
    // header. Terminals return typed instances (or `std::optional<T>`
    // / `std::vector<T>`) via `Model::hydrate(...)` -- the untyped
    // `where<T>() -> T*` / `first() -> json` API stays untouched.
    // ============================================================

    template <ModelType T> static std::unique_ptr<TypedQuery<T>> query();
    template <ModelType T> static T findAs(int id);
    template <ModelType T> static std::optional<T> tryFindAs(int id);

    // --- Fluent Interface (Chaining) ---
    Model* hasOne(ORM::OModel model, std::string fKey = "", std::string lKey = "");
    Model* belongsTo(ORM::OModel model, std::string fKey = "", std::string lKey = "");
    Model* belongsToMany(ORM::OModel model, std::string table = "", std::string fKey = "", std::string lKey = "");
    Model* hasMany(ORM::OModel model);

    Model* where(std::string field, std::string value);
    Model* where(std::string field, std::string op, std::string value);
    Model* with(ORM::OModel model);

    // --- Финални методи (ВЕЧЕ БЕЗ delete this) ---
    [[nodiscard]] json get();
    [[nodiscard]] json find(int id);
    [[nodiscard]] json findOrFail(int id);
    [[nodiscard]] json first();
    [[nodiscard]] json firstOrFail();

    // --- Non-throwing варианти (C++23 `std::expected`) ---
    [[nodiscard]] std::expected<json, DbError> tryGet() noexcept;
    [[nodiscard]] std::expected<json, DbError> tryFirst() noexcept;
    [[nodiscard]] std::expected<json, DbError> tryFind(int id) noexcept;

    void set(std::string key, json value);
    void setId(std::string id);
    std::string getId();

    Model* limit(int limit);

    void save();

    // ------------------------------------------------------------
    // Instance-level DELETE by hydrated primary key. Requires the
    // model to have a non-empty `id` (either set via `setId(...)` or
    // populated by hydration in `TypedQuery<T>` terminals). Throws
    // `std::runtime_error` if called on a fresh / unhydrated model
    // to prevent an accidental full-table wipe.
    //
    // See `GARVAN.md` "Gap 2 -- DELETE" (resolved).
    // ------------------------------------------------------------
    void remove();

    // ------------------------------------------------------------
    // Populate `attributes` and `id` from a JSON row (typically the
    // output of the connection layer after `JsonValue::parse`). Used
    // by `TypedQuery<T>` terminals for rehydration; also usable
    // directly by consumer code that already has a JSON row in hand.
    //
    // Existing attributes are cleared before the copy. Keys whose
    // values are `null` are copied as-is so callers can distinguish
    // "not present" from "explicit NULL" later.
    //
    // See `GARVAN.md` "Gap 7 -- rehydration" (resolved).
    // ------------------------------------------------------------
    void hydrate(const json& row);

    DbClient* getDb();

    // Access the underlying Builder for chain composition from the
    // typed-query wrapper. Prefer the `Model::query<T>()` /
    // `TypedQuery<T>` API in application code; direct builder access
    // is an escape hatch.
    Builder* getBuilder();

private:
    std::unique_ptr<Builder> builder;
    std::unique_ptr<DbClient> db;
    std::string id;

protected:
    json attributes;

    bool isPointer = false;
};
}

// Included at the end so the wrapper can reference the complete
// `Garvan::Model` type while its own template methods can still
// reach the `Model::hydrate/getBuilder` accessors declared above.
#include "../orm/typed_query.h"

#endif
