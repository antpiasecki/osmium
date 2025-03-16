#include "http.hh"

bool Net::s_do_verification = true; // NOLINT

Net::Response Net::get(const QUrl &url) {
  httplib::Result resp;
  httplib::Headers headers = {{"User-Agent", std::string(s_user_agent)}};

  auto host = url.host().toStdString();
  if (url.port() != -1) {
    host += ":" + std::to_string(url.port());
  }

  auto path = url.path().toStdString();
  if (path.empty()) {
    path = "/";
  }

  if (url.scheme() == "https") {
#if CPPHTTPLIB_OPENSSL_SUPPORT
    httplib::SSLClient client(host);
    client.enable_server_hostname_verification(s_do_verification);
    client.enable_server_certificate_verification(s_do_verification);
    resp = client.Get(path, headers);
#else
    return Response{false, "Osmium was built without SSL support", url,
                    std::move(resp)};
#endif
  } else if (url.scheme() == "http") {
    httplib::Client client(host);
    client.enable_server_hostname_verification(s_do_verification);
    client.enable_server_certificate_verification(s_do_verification);
    resp = client.Get(path, headers);
  } else {
    return Response{"Unsupported schema: " + url.scheme(), url,
                    std::move(resp)};
  }

  if (!resp) {
    return Response{
        "Request failed with error: " +
            QString::fromStdString(httplib::to_string(resp.error())),
        url, std::move(resp)};
  }

  if (resp->status == httplib::MovedPermanently_301 ||
      resp->status == httplib::Found_302 ||
      resp->status == httplib::TemporaryRedirect_307 ||
      resp->status == httplib::PermanentRedirect_308) {
    return Net::get(QString::fromStdString(resp->get_header_value("Location")));
  }

  if (resp->status != httplib::OK_200) {
    return Response{"Request failed with status code " +
                        QString::number(resp->status),
                    url, std::move(resp)};
  }

  return Response{"", url, std::move(resp)};
}

QUrl Net::resolve_url(const QString &url, const QString &current_url) {
  if (url.startsWith("//")) {
    return "http:" + url;
  }

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
