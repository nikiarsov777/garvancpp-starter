#pragma once
#include "crow.h"
#include <string>
#include <vector>

namespace AppServices {

// Dictionary-backed i18n for the Garvan docs site.
//
//   Languages: en | bg | ru | es | tr | pt     (default: en)
//   Storage:   cookie `lang`, 1-year Max-Age, set via GET /lang/<code>.
//
// The dictionary is loaded lazily from public/langs/<code>.json (one flat
// JSON object per language, keys already prefixed with `t_` so they map
// 1:1 to mustache tokens in the site's HTML templates).
class I18n {
public:
    // Supported language codes in dictionary order. First entry is the
    // default language (used as the ultimate fallback for missing keys).
    static const std::vector<std::string>& languages();

    // Default language ("en").
    static const std::string& defaultLang();

    // Read + validate the `lang` cookie. Returns one of languages();
    // falls back to defaultLang() if the cookie is missing or invalid.
    static std::string lang_for(const crow::request& req);

    // Build a "Set-Cookie" header value for the lang cookie
    // (Path=/, Max-Age=1y, SameSite=Lax). Unknown codes are coerced
    // to defaultLang() before serialisation.
    static std::string langCookieHeader(const std::string& lang);

    // Look up `key` for `lang`. Falls back to EN, then to the key
    // itself (so missing keys are visible in the UI).
    static std::string t(const std::string& lang, const std::string& key);

    // Populate a mustache context with:
    //   {{lang}}         -> "en" | "bg" | ...
    //   {{lang_label}}   -> "EN" | "BG" | ... (uppercased)
    //   {{is_en}}..{{is_pt}} boolean flags for the currently active lang
    //   {{js_dict}}      -> the <script>window.T={...}</script> blob
    //   {{t_XXX}}        -> every dictionary key as a mustache token
    static void inject(crow::mustache::context& ctx, const std::string& lang);

    // Build the `<script>window.T={...};</script>` blob for pages that
    // want to consume translations from JS.
    static std::string js_dict(const std::string& lang);
};

} // namespace AppServices
