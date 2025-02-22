#include "window.hh"
#include "style.hh"

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
  set_default_size(800, 600);

  auto css_provider = Gtk::CssProvider::create();
  css_provider->load_from_data(GLOBAL_STYLE.data());
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

void OsmiumWindow::navigate(const std::string &url) {
  m_current_url = url;
  m_url_entry.set_text(m_current_url);
  clear_page();

  // TODO: fix this horrible regex
  std::smatch match;
  assert(std::regex_match(m_current_url, match,
                          std::regex(R"((https?):\/\/([^\/]+)(\/.*)?)")));

  // TODO: dont use httplib
  httplib::Result resp;
  if (match[1].str() == "https") {
    httplib::SSLClient http(match[2].str());
    resp = http.Get(match[3].str());
  } else if (match[1].str() == "http") {
    httplib::Client http(match[2].str());
    resp = http.Get(match[3].str());
  } else {
    assert(false);
  }

  assert(resp->status == 200);
  m_dom = parse(resp->body);

  render(m_dom, nullptr);
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