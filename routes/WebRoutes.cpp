#include "WebRoutes.h"
#include "DocsRouter.h"
#include <string>
#include <algorithm>

using namespace std;
using namespace Routes;

// Helper: sanitize a path - strip leading/trailing slashes and any ".." segments.
static std::string sanitize_path(const std::string &raw)
{
    std::string s = raw;
    while (!s.empty() && s.front() == '/') s.erase(0, 1);
    while (!s.empty() && s.back() == '/')  s.pop_back();

    // Reject ".." traversal
    if (s.find("..") != std::string::npos) return "";
    return s;
}

WebRoutes::WebRoutes(crow::SimpleApp &app)
{
    // ----- Home -----
    CROW_ROUTE(app, "/")
    ([](const crow::request &req) {
        std::string lang = DocsRouter::detectLang(req);
        std::string body = DocsRouter::render("home", lang);
        crow::response res(body);
        res.set_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    // ----- Language switch -----
    // POST and GET both accepted (cookie + 302 redirect back to Referer / "/")
    CROW_ROUTE(app, "/lang/<string>")
    ([](const crow::request &req, std::string lang_code) {
        const auto &langs = DocsRouter::languages();
        if (std::find(langs.begin(), langs.end(), lang_code) == langs.end()) {
            lang_code = DocsRouter::defaultLang();
        }

        std::string referer = req.get_header_value("Referer");
        if (referer.empty()) referer = "/";

        crow::response res(302);
        res.set_header("Location", referer);
        res.set_header("Set-Cookie", DocsRouter::langCookieHeader(lang_code));
        return res;
    });

    // ----- Catch-all docs route -----
    // Resolves any unmatched GET path against the docs page index.
    CROW_CATCHALL_ROUTE(app)
    ([](const crow::request &req) {
        std::string lang = DocsRouter::detectLang(req);

        // Strip the leading slash off the request path
        std::string path = sanitize_path(req.url);

        // Strip query string if any
        auto qpos = path.find('?');
        if (qpos != std::string::npos) path = path.substr(0, qpos);

        const auto &P = DocsRouter::pages();
        if (P.find(path) != P.end()) {
            crow::response res(DocsRouter::render(path, lang));
            res.set_header("Content-Type", "text/html; charset=utf-8");
            return res;
        }

        crow::response res(404, DocsRouter::render404(lang));
        res.set_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });
}

WebRoutes::~WebRoutes()
{
}
