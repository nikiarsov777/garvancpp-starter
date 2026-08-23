#ifndef JSONVALUE_H
#define JSONVALUE_H
#pragma once
#include <string>
#include <string_view>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <variant>
#include <memory>
#include <cctype>
#include <cstdint>

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

    // ----------------------------------------------------------------
    // Minimal RFC-8259 JSON parser. Used primarily by the ORM's typed
    // hydration path (`Garvan::TypedQuery<T>`) to turn the RawJson
    // string returned by connection layers back into a walkable
    // Object/Array tree.  Not intended to be the fastest parser in the
    // world -- it is intended to be small, dependency-free and
    // correct for the shape of data the drivers produce (string /
    // number / bool / null / nested object|array).
    //
    // Throws std::runtime_error with a byte offset on malformed input.
    // ----------------------------------------------------------------
    static JsonValue parse(std::string_view src) {
        size_t i = 0;
        auto val = parseValue(src, i);
        skipWs(src, i);
        if (i != src.size()) {
            throw std::runtime_error("JsonValue::parse: trailing garbage at offset "
                                     + std::to_string(i));
        }
        return val;
    }

private:
    static void skipWs(std::string_view s, size_t& i) {
        while (i < s.size()) {
            char c = s[i];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') ++i;
            else break;
        }
    }

    [[noreturn]] static void parseError(const char* msg, size_t i) {
        throw std::runtime_error(std::string("JsonValue::parse: ") + msg
                                 + " at offset " + std::to_string(i));
    }

    static JsonValue parseValue(std::string_view s, size_t& i) {
        skipWs(s, i);
        if (i >= s.size()) parseError("unexpected end of input", i);
        char c = s[i];
        if (c == '"') return parseString(s, i);
        if (c == '{') return parseObject(s, i);
        if (c == '[') return parseArray(s, i);
        if (c == 't' || c == 'f') return parseBool(s, i);
        if (c == 'n') return parseNull(s, i);
        if (c == '-' || (c >= '0' && c <= '9')) return parseNumber(s, i);
        parseError("unexpected character", i);
    }

    static JsonValue parseString(std::string_view s, size_t& i) {
        if (s[i] != '"') parseError("expected '\"'", i);
        ++i;
        std::string out;
        while (i < s.size()) {
            char c = s[i++];
            if (c == '"') return JsonValue(out);
            if (c == '\\') {
                if (i >= s.size()) parseError("unterminated escape", i);
                char e = s[i++];
                switch (e) {
                    case '"':  out.push_back('"');  break;
                    case '\\': out.push_back('\\'); break;
                    case '/':  out.push_back('/');  break;
                    case 'b':  out.push_back('\b'); break;
                    case 'f':  out.push_back('\f'); break;
                    case 'n':  out.push_back('\n'); break;
                    case 'r':  out.push_back('\r'); break;
                    case 't':  out.push_back('\t'); break;
                    case 'u': {
                        if (i + 4 > s.size()) parseError("bad \\u escape", i);
                        unsigned cp = 0;
                        for (int k = 0; k < 4; ++k) {
                            char h = s[i++];
                            cp <<= 4;
                            if      (h >= '0' && h <= '9') cp |= (h - '0');
                            else if (h >= 'a' && h <= 'f') cp |= (h - 'a' + 10);
                            else if (h >= 'A' && h <= 'F') cp |= (h - 'A' + 10);
                            else parseError("invalid hex in \\u escape", i);
                        }
                        // UTF-8 encode (BMP only; surrogates as raw)
                        if (cp < 0x80) {
                            out.push_back(static_cast<char>(cp));
                        } else if (cp < 0x800) {
                            out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
                            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                        } else {
                            out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
                            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                        }
                        break;
                    }
                    default: parseError("bad escape character", i);
                }
            } else {
                out.push_back(c);
            }
        }
        parseError("unterminated string", i);
    }

    static JsonValue parseNumber(std::string_view s, size_t& i) {
        size_t start = i;
        if (s[i] == '-') ++i;
        while (i < s.size() && s[i] >= '0' && s[i] <= '9') ++i;
        bool isFloat = false;
        if (i < s.size() && s[i] == '.') {
            isFloat = true; ++i;
            while (i < s.size() && s[i] >= '0' && s[i] <= '9') ++i;
        }
        if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
            isFloat = true; ++i;
            if (i < s.size() && (s[i] == '+' || s[i] == '-')) ++i;
            while (i < s.size() && s[i] >= '0' && s[i] <= '9') ++i;
        }
        std::string tok(s.substr(start, i - start));
        if (isFloat) {
            return JsonValue(std::stod(tok));
        }
        try {
            return JsonValue(static_cast<int64_t>(std::stoll(tok)));
        } catch (...) {
            // fall back to double for out-of-range integers
            return JsonValue(std::stod(tok));
        }
    }

    static JsonValue parseBool(std::string_view s, size_t& i) {
        if (s.compare(i, 4, "true") == 0)  { i += 4; return JsonValue(true); }
        if (s.compare(i, 5, "false") == 0) { i += 5; return JsonValue(false); }
        parseError("invalid literal (expected true/false)", i);
    }

    static JsonValue parseNull(std::string_view s, size_t& i) {
        if (s.compare(i, 4, "null") == 0)  { i += 4; return JsonValue(nullptr); }
        parseError("invalid literal (expected null)", i);
    }

    static JsonValue parseArray(std::string_view s, size_t& i) {
        ++i; // consume '['
        JsonValue arr; arr.value = Array{};
        skipWs(s, i);
        if (i < s.size() && s[i] == ']') { ++i; return arr; }
        while (true) {
            arr.push_back(parseValue(s, i));
            skipWs(s, i);
            if (i >= s.size()) parseError("unterminated array", i);
            if (s[i] == ',') { ++i; skipWs(s, i); continue; }
            if (s[i] == ']') { ++i; return arr; }
            parseError("expected ',' or ']' in array", i);
        }
    }

    static JsonValue parseObject(std::string_view s, size_t& i) {
        ++i; // consume '{'
        JsonValue obj; obj.value = Object{};
        skipWs(s, i);
        if (i < s.size() && s[i] == '}') { ++i; return obj; }
        while (true) {
            skipWs(s, i);
            JsonValue key = parseString(s, i);
            skipWs(s, i);
            if (i >= s.size() || s[i] != ':') parseError("expected ':' in object", i);
            ++i;
            JsonValue val = parseValue(s, i);
            std::get<Object>(obj.value).emplace(key.asString(), std::move(val));
            skipWs(s, i);
            if (i >= s.size()) parseError("unterminated object", i);
            if (s[i] == ',') { ++i; continue; }
            if (s[i] == '}') { ++i; return obj; }
            parseError("expected ',' or '}' in object", i);
        }
    }

public:

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
