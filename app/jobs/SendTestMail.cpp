#include "SendTestMail.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sstream>
#include <string>
#include <unistd.h>

#include <curl/curl.h>

#include "tools/Helper.h"

using namespace AppJobs;

namespace {

// Payload upload cursor за CURLOPT_READFUNCTION.
struct UploadCtx {
    const std::string* data;
    std::size_t pos;
};

std::size_t readCallback(char* buffer, std::size_t size, std::size_t nitems, void* userp)
{
    auto* ctx = static_cast<UploadCtx*>(userp);
    const std::size_t remaining = ctx->data->size() - ctx->pos;
    const std::size_t want = size * nitems;
    const std::size_t n = std::min(want, remaining);
    if (n == 0) return 0;
    std::memcpy(buffer, ctx->data->data() + ctx->pos, n);
    ctx->pos += n;
    return n;
}

// .env стойностите често идват в двойни кавички -- махаме ги
// преди да минат в SMTP хедъри/URL.
std::string stripQuotes(std::string s)
{
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.size() - 2);
    return s;
}

std::string rfc2822Date()
{
    std::time_t t = std::time(nullptr);
    std::tm tm{};
    gmtime_r(&t, &tm);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S +0000", &tm);
    return std::string(buf);
}

} // namespace

void SendTestMail::handle()
{
    // MAIL_* конфигурация от .env. Празни стойности -> "<unset>" за лог,
    // но за реален SMTP send гърмим рано с ясен диагностичен ред.
    Garvan::Helper helper;
    auto env = [&](const char* k) { return stripQuotes(helper.getenv(k)); };

    const std::string host     = env("MAIL_HOST");
    const std::string port     = env("MAIL_PORT");
    const std::string user     = env("MAIL_USERNAME");
    const std::string pass     = env("MAIL_PASSWORD");
    const std::string from     = env("MAIL_FROM_ADDRESS");
    const std::string fromName = env("MAIL_FROM_NAME");
    const std::string enc      = env("MAIL_ENCRYPTION");
    const std::string driver   = env("MAIL_DRIVER");

    std::fprintf(stdout,
        "[SendTestMail] dispatch driver=%s host=%s:%s enc=%s from=<%s> "
        "-> to=<%s> subject=\"%s\" body_len=%zu\n",
        driver.c_str(), host.c_str(), port.c_str(), enc.c_str(),
        from.c_str(), to_.c_str(), subject_.c_str(), body_.size());
    std::fflush(stdout);

    if (host.empty() || port.empty() || from.empty() || to_.empty()) {
        std::fprintf(stderr,
            "[SendTestMail] ERROR: missing config -- host/port/from/to must be set\n");
        std::fflush(stderr);
        return;
    }

    // Port 465 = implicit TLS (SMTPS). 587 = STARTTLS. За другите port-ове
    // ползваме encryption хинт от .env ("ssl"/"smtps" -> implicit, иначе
    // STARTTLS чрез CURLUSESSL_ALL).
    const bool implicitTls =
        (port == "465") || (enc == "ssl") || (enc == "smtps");
    const std::string url =
        (implicitTls ? "smtps://" : "smtp://") + host + ":" + port;

    // RFC 822 message с HTML body (charset=UTF-8, 8bit CTE).
    const std::string msgId =
        "<" + std::to_string(static_cast<long long>(std::time(nullptr))) + "." +
        std::to_string(static_cast<long long>(::getpid())) + "@" + host + ">";

    std::ostringstream msg;
    msg << "Date: " << rfc2822Date() << "\r\n";
    msg << "To: <" << to_ << ">\r\n";
    if (!fromName.empty())
        msg << "From: \"" << fromName << "\" <" << from << ">\r\n";
    else
        msg << "From: <" << from << ">\r\n";
    msg << "Message-ID: " << msgId << "\r\n";
    msg << "Subject: " << subject_ << "\r\n";
    msg << "MIME-Version: 1.0\r\n";
    msg << "Content-Type: text/html; charset=UTF-8\r\n";
    msg << "Content-Transfer-Encoding: 8bit\r\n";
    msg << "\r\n";
    msg << body_ << "\r\n";
    const std::string payload = msg.str();

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::fprintf(stderr, "[SendTestMail] ERROR: curl_easy_init failed\n");
        std::fflush(stderr);
        return;
    }

    struct curl_slist* rcpts = nullptr;
    UploadCtx ctx{&payload, 0};

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (!user.empty())
        curl_easy_setopt(curl, CURLOPT_USERNAME, user.c_str());
    if (!pass.empty())
        curl_easy_setopt(curl, CURLOPT_PASSWORD, pass.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, static_cast<long>(CURLUSESSL_ALL));
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, ("<" + from + ">").c_str());

    const std::string rcpt = "<" + to_ + ">";
    rcpts = curl_slist_append(rcpts, rcpt.c_str());
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, rcpts);

    curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15L);
    // Set to 1L при debug на TLS handshake / SMTP dialog.
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

    const CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::fprintf(stderr,
            "[SendTestMail] SMTP send FAILED via %s: %s\n",
            url.c_str(), curl_easy_strerror(res));
    } else {
        std::fprintf(stdout,
            "[SendTestMail] SMTP send OK via %s -> <%s>\n",
            url.c_str(), to_.c_str());
    }
    std::fflush(stdout);
    std::fflush(stderr);

    curl_slist_free_all(rcpts);
    curl_easy_cleanup(curl);
}

Garvan::JsonValue SendTestMail::payload() const
{
    auto p = Garvan::JsonValue::Object();
    p["to"]      = to_;
    p["subject"] = subject_;
    p["body"]    = body_;
    return p;
}

std::unique_ptr<Garvan::Job> SendTestMail::fromPayload(const Garvan::JsonValue& p)
{
    // Phase C: JsonValue::parse ще ни даде реални стойности тук.
    // Засега (без parser) правим best-effort -- работи, ако caller-ът
    // подаде живия JsonValue (тестове, in-memory queue). За database
    // driver -- ще се пренапише в Phase C.
    auto get = [&](const char* k) -> std::string {
        try {
            auto v = const_cast<Garvan::JsonValue&>(p)[k];
            return v.asString();
        } catch (...) { return {}; }
    };
    return std::make_unique<SendTestMail>(get("to"), get("subject"), get("body"));
}
