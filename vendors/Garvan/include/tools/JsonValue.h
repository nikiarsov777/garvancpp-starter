#ifndef JSONVALUE_H
#define JSONVALUE_H
#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <variant>
#include <memory>

class JsonValue {
public:
    struct RawJson {
        std::string value;
    };
    using Object = std::unordered_map<std::string, JsonValue>;
    using Array = std::vector<JsonValue>;
    using ValueType = std::variant<std::nullptr_t, bool, int64_t, double, std::string, Array, Object, RawJson>;

private:
    ValueType value;

public:
    // Constructors
    JsonValue() : value(nullptr) {}
    JsonValue(std::nullptr_t) : value(nullptr) {}
    JsonValue(bool b) : value(b) {}
    JsonValue(int64_t i) : value(i) {}
    JsonValue(int i) : value((int64_t)i) {}
    JsonValue(double d) : value(d) {}
    JsonValue(long double d) : value((double)d) {}
    JsonValue(long long v) : value((int64_t)v) {}
    JsonValue(const std::string& s) : value(s) {}
    JsonValue(const char* s) : value(std::string(s)) {}
    JsonValue(const Array& a) : value(a) {}
    JsonValue(const Object& o) : value(o) {}
    JsonValue(const RawJson& r) : value(r) {}

    // Accessors
    bool isNull() const { return std::holds_alternative<std::nullptr_t>(value); }
    bool isBool() const { return std::holds_alternative<bool>(value); }
    bool isInt() const { return std::holds_alternative<int64_t>(value); }
    bool isDouble() const { return std::holds_alternative<double>(value); }
    bool isString() const { return std::holds_alternative<std::string>(value); }
    bool isArray() const { return std::holds_alternative<Array>(value); }
    bool isObject() const { return std::holds_alternative<Object>(value); }

    JsonValue& operator=(const RawJson& r) {
        value = r;
        return *this;
    }

    // Object access
    JsonValue& operator[](const std::string& key) {
        if (!isObject()) value = Object{};
        return std::get<Object>(value)[key];
    }

    const JsonValue& operator[](const std::string& key) const {
        static JsonValue nullVal;
        if (!isObject()) return nullVal;
        const auto& obj = std::get<Object>(value);
        auto it = obj.find(key);
        if (it != obj.end()) return it->second;
        return nullVal;
    }

    // Array access
    void push_back(const JsonValue& v) {
        if (!isArray()) value = Array{};
        std::get<Array>(value).push_back(v);
    }

    // ----------------------------------------------------------------
    // Serialize to a JSON string per RFC 8259.  Strings and object keys
    // MUST escape `"`, `\` and control characters U+0000..U+001F or the
    // resulting JSON is malformed and (worse) re-parseable into a
    // structure the caller did not intend -- which is a direct injection
    // vector when the dump output is fed to a JSON parser at the
    // backend (e.g. bsoncxx::from_json in the Mongo path).
    // ----------------------------------------------------------------
    static std::string escapeJsonString(const std::string& s) {
        std::string out;
        out.reserve(s.size() + 2);
        for (unsigned char c : s) {
            switch (c) {
                case '"':  out.append("\\\""); break;
                case '\\': out.append("\\\\"); break;
                case '\b': out.append("\\b");  break;
                case '\f': out.append("\\f");  break;
                case '\n': out.append("\\n");  break;
                case '\r': out.append("\\r");  break;
                case '\t': out.append("\\t");  break;
                default:
                    if (c < 0x20) {
                        static const char hex[] = "0123456789abcdef";
                        out.append("\\u00");
                        out.push_back(hex[(c >> 4) & 0xF]);
                        out.push_back(hex[c & 0xF]);
                    } else {
                        out.push_back(static_cast<char>(c));
                    }
            }
        }
        return out;
    }

    std::string dump() const {
        if (isNull()) return "null";
        if (std::holds_alternative<RawJson>(value)) {
            return std::get<RawJson>(value).value;
        }
        if (isBool()) return std::get<bool>(value) ? "true" : "false";
        if (isInt()) return std::to_string(std::get<int64_t>(value));
        if (isDouble()) return std::to_string(std::get<double>(value));
        if (isString())
            return "\"" + escapeJsonString(std::get<std::string>(value)) + "\"";
        if (isArray()) {
            std::string s = "[";
            bool first = true;
            for (auto& v : std::get<Array>(value)) {
                if (!first) s += ",";
                s += v.dump();
                first = false;
            }
            s += "]";
            return s;
        }
        if (isObject()) {
            std::string s = "{";
            bool first = true;
            for (auto& [k,v] : std::get<Object>(value)) {
                if (!first) s += ",";
                s += "\"" + escapeJsonString(k) + "\":" + v.dump();
                first = false;
            }
            s += "}";
            return s;
        }
        return "null";
    }

    size_t size() const {
        if (std::holds_alternative<Object>(value)) {
            return std::get<Object>(value).size();
        }
        if (std::holds_alternative<Array>(value)) {
            return std::get<Array>(value).size();
        }
        return 0;
    }

    const std::string& asString() const {
        if (!std::holds_alternative<std::string>(value))
            throw std::runtime_error("JsonValue is not a string");

        return std::get<std::string>(value);
    }

    const Object& asObject() const{
        return std::get<Object>(value);
    }
    const Array& asArray() const {
        return std::get<Array>(value);
    }

    int asInt() const {
        if (std::holds_alternative<int64_t>(value))
            return static_cast<int>(std::get<int64_t>(value));

        if (std::holds_alternative<double>(value))
            return static_cast<int>(std::get<double>(value));

        throw std::runtime_error("JsonValue is not a number");
    }

    bool empty() const {
        if (std::holds_alternative<Object>(value))
            return std::get<Object>(value).empty();

        if (std::holds_alternative<Array>(value))
            return std::get<Array>(value).empty();

        if (std::holds_alternative<std::string>(value))
            return std::get<std::string>(value).empty();

        if (std::holds_alternative<std::nullptr_t>(value))
            return true;

        return false;
    }
};

// Convenience alias — позволява `Garvan::JsonValue` в новите API
// (queue/, events/), паралелно със стария глобален `JsonValue`.
namespace Garvan { using JsonValue = ::JsonValue; }

#endif // JSONVALUE_H
