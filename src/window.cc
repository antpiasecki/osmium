#include "window.hh"
#include "style.hh"
#include <ada.h>

// stolen from stackoverflow. i love c++
static void trim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return std::isspace(ch) == 0;
          }));
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return std::isspace(ch) == 0; })
              .base(),
          s.end());
}

OsmiumWindow::OsmiumWindow() {
  set_title("Osmium");
  set_default_size(1024, 768);

  auto css_provider = Gtk::CssProvider::create();
  css_provider->load_from_data(std::string(GLOBAL_STYLE));
  Gtk::StyleContext::add_provider_for_display(
      Gdk::Display::get_default(), css_provider,
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  m_main_layout.set_orientation(Gtk::Orientation::VERTICAL);
  set_child(m_main_layout);

  m_url_entry.set_hexpand(true);
  m_url_entry.signal_activate().connect(
      [this]() { navigate(m_url_entry.get_text()); });
  m_navbar_layout.append(m_url_entry);

  m_go_button.signal_clicked().connect(
      [this]() { navigate(m_url_entry.get_text()); });
  m_navbar_layout.append(m_go_button);

  m_main_layout.append(m_navbar_layout);

  m_main_layout.append(m_page_scrolled_window);

  m_page_layout.set_orientation(Gtk::Orientation::VERTICAL);
  m_page_layout.set_halign(Gtk::Align::START);
  m_page_layout.set_expand(true);
  m_page_layout.set_margin(10);
  m_page_scrolled_window.set_child(m_page_layout);

  navigate("http://example.org/");

  present();
}

// TODO: the arg probably shouldnt be named url
void OsmiumWindow::navigate(std::string url) {
  auto start_time = duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch());

  // handle absolute paths (/home, /static/style.css)
  if (url.starts_with('/')) {
    auto parsed_current_url = ada::parse(m_current_url);
    if (!parsed_current_url) {
      log("Failed to parse URL: " + url + ".");
      return;
    }

    parsed_current_url->set_pathname(url);

    url = parsed_current_url->get_href();
  }
  // TODO: handle relative paths

  auto parsed_url = ada::parse(url);
  if (!parsed_url) {
    log("Failed to parse URL: " + url + ".");
    return;
  }

  m_current_url = url;
  m_url_entry.set_text(url);
  clear_page();

  log("Navigating to " + url + "...");

  // TODO: dont use httplib
  httplib::Result resp;
  httplib::Headers headers = {{"User-Agent", std::string(s_user_agent)}};

  if (parsed_url->get_protocol() == "https:") {
    httplib::SSLClient client(std::string(parsed_url->get_host()));
    resp = client.Get(std::string(parsed_url->get_pathname()), headers);
  } else if (parsed_url->get_protocol() == "http:") {
    httplib::Client client(std::string(parsed_url->get_host()));
    resp = client.Get(std::string(parsed_url->get_pathname()), headers);
  } else {
    log("Unsupported protocol: " + std::string(parsed_url->get_protocol()) +
        ".");
    return;
  }

  if (!resp) {
    log("Request failed with error: " + httplib::to_string(resp.error()) + ".");
    return;
  }

  if (resp->status == 301 || resp->status == 302 || resp->status == 307 ||
      resp->status == 308) {
    navigate(resp->get_header_value("Location"));
    return;
  }

  if (resp->status != 200) {
    log("Request failed with status code " + std::to_string(resp->status) +
        ".");
    return;
  }

  log("Parsing " + std::to_string(resp->body.length() / 1024) + " KBs...");
  m_dom = parse(resp->body);

  log("Rendering...");
  render(m_dom, nullptr);

  auto end_time = duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch());

  log("Rendered " + std::to_string(m_page_widgets.size()) + " widgets.");
  log("Done in " + std::to_string((end_time - start_time).count()) + " ms.");
}

void OsmiumWindow::render_element(const ElementPtr &el,
                                  const ElementPtr & /*parent*/) {
  for (const auto &child : el->children()) {
    render(child, el);
  }
}

void OsmiumWindow::render_textnode(const TextNodePtr &textnode,
                                   const ElementPtr &parent) {
  if (parent != nullptr &&
      (parent->name() == "script" || parent->name() == "style" ||
       parent->name() == "head")) {
    return;
  }

  auto content = textnode->content();
  trim(content);
  if (content.empty()) {
    return;
  }

  if (parent != nullptr && parent->name() == "title") {
    m_page_title = content;
    set_title(m_page_title + " - Osmium");
    return;
  }

  if (parent != nullptr && parent->name() == "a") {
    auto *label = Gtk::make_managed<Gtk::LinkButton>();
    label->set_label(content);

    auto href = parent->attributes()["href"];
    label->signal_activate_link().connect(
        [this, href]() {
          navigate(href);
          return true;
        },
        false);

    append_widget(label);
    return;
  }

  auto *label = Gtk::make_managed<Gtk::Label>(content);
  label->set_wrap(true);
  label->set_xalign(0.0);

  if (parent != nullptr && parent->is_heading()) {
    label->get_style_context()->add_class(parent->name());
  }
  if (parent != nullptr && parent->name() == "p") {
    label->get_style_context()->add_class("p");
  }

  append_widget(label);
}

void OsmiumWindow::log(const std::string &s) { std::cout << s << std::endl; }