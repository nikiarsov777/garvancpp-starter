#include "I18n.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <unordered_map>

using namespace AppServices;

// ---------------------------------------------------------------------------
// Language table. Order matches the popup site (EN,BG,RU,ES,TR,PT) so the
// two projects can share translation tooling. Index 0 is default + fallback.
// ---------------------------------------------------------------------------
static const char* kLangOrder[] = { "en", "bg", "ru", "es", "tr", "pt" };
static constexpr int LANG_COUNT = 6;
static constexpr int L_EN      = 0;
static constexpr int L_DEFAULT = L_EN;

static int lang_index(const std::string& lang) {
    for (int i = 0; i < LANG_COUNT; ++i)
        if (lang == kLangOrder[i]) return i;
    return L_DEFAULT;
}

const std::vector<std::string>& I18n::languages() {
    static const std::vector<std::string> v = {
        kLangOrder[0], kLangOrder[1], kLangOrder[2],
        kLangOrder[3], kLangOrder[4], kLangOrder[5],
    };
    return v;
}

const std::string& I18n::defaultLang() {
    static const std::string d = kLangOrder[L_DEFAULT];
    return d;
}

// A single row = one translation key, LANG_COUNT strings.
using Row  = std::array<std::string, LANG_COUNT>;
using Dict = std::unordered_map<std::string, Row>;

// ---------------------------------------------------------------------------
// Lazy dictionary loader. Reads public/langs/<code>.json for every
// supported language. Tries several candidate roots so the binary works
// whether started from the project root, from build/, or from bin/.
// ---------------------------------------------------------------------------
static const Dict& dict() {
    static Dict d;
    static std::once_flag once;
    std::call_once(once, [] {
        const char* candidates[] = {
            "public/langs",
            "../public/langs",
            "../../public/langs",
        };
        std::string dir;
        for (const char* c : candidates) {
            std::ifstream probe(std::string(c) + "/en.json");
            if (probe.good()) { dir = c; break; }
        }
        if (dir.empty()) {
            std::cerr << "[I18n] cannot locate public/langs/*.json" << std::endl;
            return;
        }
        for (int idx = 0; idx < LANG_COUNT; ++idx) {
            std::string path = dir + "/" + kLangOrder[idx] + ".json";
            std::ifstream in(path);
            if (!in.good()) {
                std::cerr << "[I18n] missing " << path << std::endl;
                continue;
            }
            std::ostringstream buf; buf << in.rdbuf();
            auto js = crow::json::load(buf.str());
            if (!js) {
                std::cerr << "[I18n] bad JSON: " << path << std::endl;
                continue;
            }
            for (const auto& kv : js) {
                std::string key = kv.key();
                auto it = d.find(key);
                if (it == d.end()) {
                    Row r; r.fill(std::string());
                    it = d.emplace(key, r).first;
                }
                it->second[idx] = kv.s();
            }
        }
    });
    return d;
}

// ---------------------------------------------------------------------------
// Cookie parsing. Small, self-contained.
// ---------------------------------------------------------------------------
static std::string cookie_value(const crow::request& req, const std::string& name) {
    std::string cookie = req.get_header_value("Cookie");
    size_t i = 0;
    while (i < cookie.size()) {
        while (i < cookie.size() && (cookie[i] == ' ' || cookie[i] == ';')) ++i;
        size_t eq = cookie.find('=', i);
        if (eq == std::string::npos) break;
        std::string k = cookie.substr(i, eq - i);
        size_t end = cookie.find(';', eq);
        std::string v = cookie.substr(eq + 1,
                                     end == std::string::npos ? std::string::npos
                                                              : end - eq - 1);
        while (!v.empty() && (v.front() == ' ' || v.front() == '\t')) v.erase(0, 1);
        while (!v.empty() && (v.back()  == ' ' || v.back()  == '\t')) v.pop_back();
        if (k == name) return v;
        if (end == std::string::npos) break;
        i = end + 1;
    }
    return "";
}

std::string I18n::lang_for(const crow::request& req) {
    std::string v = cookie_value(req, "lang");
    for (int i = 0; i < LANG_COUNT; ++i)
        if (v == kLangOrder[i]) return v;
    return kLangOrder[L_DEFAULT];
}

std::string I18n::langCookieHeader(const std::string& lang) {
    std::string l = lang;
    bool ok = false;
    for (int i = 0; i < LANG_COUNT; ++i)
        if (l == kLangOrder[i]) { ok = true; break; }
    if (!ok) l = kLangOrder[L_DEFAULT];
    return "lang=" + l + "; Path=/; Max-Age=31536000; SameSite=Lax";
}

std::string I18n::t(const std::string& lang, const std::string& key) {
    const auto& d = dict();
    auto it = d.find(key);
    if (it == d.end()) return key;
    const Row& row = it->second;
    int idx = lang_index(lang);
    if (!row[idx].empty())   return row[idx];
    if (!row[L_EN].empty())  return row[L_EN];
    return key;
}

// Small JSON-string escaper for js_dict.
static std::string json_esc(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (c < 0x20) {
                    char buf[8]; snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out.push_back((char)c);
                }
        }
    }
    return out;
}

std::string I18n::js_dict(const std::string& lang) {
    const auto& d = dict();
    int idx = lang_index(lang);
    std::ostringstream o;
    o << "<script>window.T={";
    bool first = true;
    for (const auto& kv : d) {
        const Row& row = kv.second;
        std::string v = !row[idx].empty() ? row[idx]
                      : !row[L_EN].empty() ? row[L_EN]
                      : kv.first;
        if (!first) o << ',';
        first = false;
        o << '"' << json_esc(kv.first) << "\":\"" << json_esc(v) << '"';
    }
    if (!first) o << ',';
    o << "lang:\"" << json_esc(lang) << "\"};</script>";
    return o.str();
}

void I18n::inject(crow::mustache::context& ctx, const std::string& lang) {
    ctx["lang"] = lang;
    std::string upper = lang;
    for (auto& c : upper) c = (char)std::toupper((unsigned char)c);
    ctx["lang_label"] = upper;
    ctx["is_en"] = (lang == "en");
    ctx["is_bg"] = (lang == "bg");
    ctx["is_ru"] = (lang == "ru");
    ctx["is_es"] = (lang == "es");
    ctx["is_tr"] = (lang == "tr");
    ctx["is_pt"] = (lang == "pt");

    const auto& d = dict();
    int idx = lang_index(lang);
    for (const auto& kv : d) {
        const Row& row = kv.second;
        std::string val = !row[idx].empty() ? row[idx]
                        : !row[L_EN].empty() ? row[L_EN]
                        : kv.first;
        ctx[kv.first] = val;
    }

    ctx["js_dict"] = js_dict(lang);
}
