#include "net.hh"

Net::Response Net::get(const QUrl &url, bool do_verification) {
  httplib::Result resp;
  httplib::Headers headers = {{"User-Agent", std::string(s_user_agent)}};

  auto host = url.host().toStdString();
  auto path = url.path().toStdString();
  if (path.empty()) {
    path = "/";
  }

  if (url.scheme() == "https") {
#if CPPHTTPLIB_OPENSSL_SUPPORT
    httplib::SSLClient client(host);
    client.enable_server_hostname_verification(do_verification);
    client.enable_server_certificate_verification(do_verification);
    resp = client.Get(path, headers);
#else
    return Response{false, "Osmium was built without SSL support", url,
                    std::move(resp)};
#endif
  } else if (url.scheme() == "http") {
    httplib::Client client(host);
    client.enable_server_hostname_verification(do_verification);
    client.enable_server_certificate_verification(do_verification);
    resp = client.Get(path, headers);
  } else {
    return Response{false, "Unsupported schema: " + url.scheme(), url,
                    std::move(resp)};
  }

  if (!resp) {
    return Response{
        false,
        "Request failed with error: " +
            QString::fromStdString(httplib::to_string(resp.error())),
        url, std::move(resp)};
  }

  if (resp->status == 301 || resp->status == 302 || resp->status == 307 ||
      resp->status == 308) {
    return Net::get(QString::fromStdString(resp->get_header_value("Location")),
                    do_verification);
  }

  if (resp->status != 200) {
    return Response{false,
                    "Request failed with status code " +
                        QString::number(resp->status),
                    url, std::move(resp)};
  }

  return Response{true, "", url, std::move(resp)};
}

QUrl Net::resolve_url(const QString &url, const QString &current_url) {
  if (url.startsWith('/')) {
    QUrl parsed_current_url(current_url);
    if (!parsed_current_url.isValid()) {
      return QUrl{};
    }

    parsed_current_url.setPath(url);
    return parsed_current_url;
  }

  if (!url.contains("://")) {
    QUrl parsed_current_url(current_url);
    if (!parsed_current_url.isValid()) {
      return QUrl{};
    }

    parsed_current_url.setPath("/" + url);
    return parsed_current_url;
  }

  QUrl parsed_url(url);
  if (!parsed_url.isValid()) {
    return QUrl{};
  }

  return url;
}
