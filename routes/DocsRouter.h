#ifndef DOCSROUTER_H
#define DOCSROUTER_H

#pragma once

#include "crow.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace Routes
{
    struct TocItem
    {
        std::string anchor;
        std::string label_en;
        std::string label_bg;
        bool sub = false;
    };

    struct PageMeta
    {
        std::string key;         // e.g. "guides/routes"
        std::string section;     // home | getting_started | guides | garvan | reference
        std::string title_en;
        std::string title_bg;
        std::string desc_en;
        std::string desc_bg;
        std::vector<TocItem> toc;
        std::string prev_key;
        std::string next_key;
    };

    class DocsRouter
    {
    public:
        // Supported language codes
        static const std::vector<std::string> &languages();

        // Page metadata index keyed by URL path (without leading slash). "" = home.
        static const std::unordered_map<std::string, PageMeta> &pages();

        // Default language used when cookie missing or fallback needed.
        static const std::string &defaultLang();

        // Read `lang` cookie from the request, validate; fall back to defaultLang().
        static std::string detectLang(const crow::request &req);

        // Build the full mustache context for a given page key + language.
        // Loads the content fragment (with fallback to default lang) and embeds it.
        static crow::mustache::context buildContext(const std::string &page_key,
                                                    const std::string &lang);

        // Render a page: resolves key, builds context, renders _layout.html.
        // Returns the rendered HTML.
        static std::string render(const std::string &page_key,
                                  const std::string &lang);

        // Render the 404 page for a given language.
        static std::string render404(const std::string &lang);

        // Helper: get a translated string with EN fallback.
        static std::string tr(const std::string &key, const std::string &lang);

        // Helper: build "Set-Cookie" header value for the lang cookie.
        static std::string langCookieHeader(const std::string &lang);
    };
}

#endif
