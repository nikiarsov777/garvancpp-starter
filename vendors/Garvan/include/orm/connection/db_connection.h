#pragma once
#include <string>
#include <cstdio>
#include "tools/JsonValue.h"
#include "../grammar/grammar.h"   // PreparedStatement

using json = JsonValue;
using namespace std;

class DbConnection {
public:
    virtual ~DbConnection() = default;

    // ---------------------------------------------------------------
    // The new safe path: execute a PreparedStatement carrying SQL with
    // placeholders + bound params. Each backend implements this with
    // its native prepared-statement / parameter-binding API.
    // ---------------------------------------------------------------
    virtual json execute(const PreparedStatement& stmt) = 0;

    // Legacy raw-SQL path. Kept for backends that need to run admin /
    // schema queries without going through the Grammar pipeline.
    // The ORM Builder no longer uses this directly.
    virtual json execute(string query) = 0;

    virtual void disconnect() = 0;

    // Новите методи за Mongo поддръжка в GarvanCpp
    // virtual json executeMongo(string collection, string filterJson, int limit) = 0;

protected:
    // Escape an arbitrary C string for safe inclusion as the contents
    // of a JSON string literal. Used by connections that interpolate
    // driver-supplied error messages into JSON. Without this, an error
    // containing a quote or backslash produces malformed JSON.
    static string jsonEscape(const char* s)
    {
        string out;
        if (!s) return out;
        for (const char* p = s; *p; ++p) {
            unsigned char c = static_cast<unsigned char>(*p);
            switch (c) {
            case '"':  out.append("\\\""); break;
            case '\\': out.append("\\\\"); break;
            case '\n': out.append("\\n");  break;
            case '\r': out.append("\\r");  break;
            case '\t': out.append("\\t");  break;
            case '\b': out.append("\\b");  break;
            case '\f': out.append("\\f");  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out.append(buf);
                } else {
                    out.push_back(static_cast<char>(c));
                }
            }
        }
        return out;
    }

    static string jsonEscape(const string& s) { return jsonEscape(s.c_str()); }
};
