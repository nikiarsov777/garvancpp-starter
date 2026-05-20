#ifndef BASE_MODEL_H
#define BASE_MODEL_H

#include "../orm/omodel.h"
#include "../orm/builder.h"
#include "../db/DbClient.h"
#include <memory>
#include <string>
#include <iostream>

namespace Garvan
{
class Model : public ORM::OModel
{
public:
    explicit Model(std::string client = "PSQL");
    virtual ~Model(); // Виртуален за правилно изтриване на наследници

    void init();

    // --- СТАТИЧНИ МЕТОДИ (ВРЪЩАТ УМЕН УКАЗАТЕЛ) ---
    template <typename T>
    static std::unique_ptr<T> fastWhere(std::string field, std::string value) {
        auto instance = std::make_unique<T>();
        instance->isPointer = true;
        instance->init();
        instance->where(field, value);
        return instance;
    }

    template <typename T>
    static std::unique_ptr<T> fastWhere(std::string field, std::string op, std::string value) {
        auto instance = std::make_unique<T>();
        instance->isPointer = true;
        instance->init();
        instance->where(field, op, value);
        return instance;
    }

    template <typename T>
    static T* where(std::string field, std::string value) {
        T* instance = new T(); // Създаваме обекта
        instance->init();
        instance->where(field, value);
        return instance; // Връщаме указател за chaining
    }

    template <typename T>
    static T* where(std::string field, std::string op, std::string value) {
        T* instance = new T(); // Създаваме обекта
        instance->init();
        instance->where(field, op, value);
        return instance; // Връщаме указател за chaining
    }

    template <typename T>
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

    template <typename T>
    static json get() {
        T instance;
        instance.isPointer = true;   // see comment in find<T>() above
        instance.init();
        return instance.get();
    }

    // --- Fluent Interface (Chaining) ---
    Model* hasOne(ORM::OModel model, std::string fKey = "", std::string lKey = "");
    Model* belongsTo(ORM::OModel model, std::string fKey = "", std::string lKey = "");
    Model* belongsToMany(ORM::OModel model, std::string table = "", std::string fKey = "", std::string lKey = "");
    Model* hasMany(ORM::OModel model);

    Model* where(std::string field, std::string value);
    Model* where(std::string field, std::string op, std::string value);
    Model* with(ORM::OModel model);

    // --- Финални методи (ВЕЧЕ БЕЗ delete this) ---
    json get();
    json find(int id);
    json findOrFail(int id);
    json first();
    json firstOrFail();

    void set(std::string key, json value);
    void setId(std::string id);
    std::string getId();

    Model* limit(int limit);

    void save();

    DbClient* getDb();

private:
    std::unique_ptr<Builder> builder;
    std::unique_ptr<DbClient> db;
    std::string id;

protected:
    json attributes;

    bool isPointer = false;
};
}

#endif
