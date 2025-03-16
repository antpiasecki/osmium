#pragma once
#include "qurl.h"
#include <QUrl>
#include <httplib.h>

namespace Net {
#if defined(_WIN32) || defined(_WIN64)
static constexpr const char *s_user_agent =
    "Mozilla/5.0 (Windows NT; Win64; x64) osmium-html/0.1 Osmium/0.1";
#elif defined(__APPLE__) || defined(__MACH__)
static constexpr const char *s_user_agent =
    "Mozilla/5.0 (Macintosh) osmium-html/0.1 Osmium/0.1";
#elif defined(__linux__)
static constexpr const char *s_user_agent =
    "Mozilla/5.0 (Linux x86_64) osmium-html/0.1 Osmium/0.1";
#else
static constexpr const char *s_user_agent =
    "Mozilla/5.0 (Unknown) osmium-html/0.1 Osmium/0.1";
#endif

extern bool s_do_verification; // NOLINT

struct Response {
  QString error;
  QUrl url;
  httplib::Result result;
};

Response get(const QUrl &url);

QUrl resolve_url(const QString &url, const QString &current_url);

}; // namespace Net