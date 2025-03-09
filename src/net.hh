#pragma once
#include "qurl.h"
#include <QUrl>
#include <httplib.h>

namespace Net {
// TODO: what if we're not on linux
static constexpr const char *s_user_agent =
    "Mozilla/5.0 (Linux x86_64) osmium-html/0.1 Osmium/0.1";

struct Response {
  bool ok;
  QString error;
  QUrl url;
  httplib::Result result;
};

Response get(const QUrl &url, bool do_verification);

QUrl resolve_url(const QString &url, const QString &current_url);

}; // namespace Net