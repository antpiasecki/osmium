#include "style.hh"
#include <cassert>
#include <gtkmm.h>
#include <httplib.h>
#include <memory>
#include <osmium-html/parser.hh>
#include <string>

// stolen from stackoverflow. i love c++
inline void trim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return std::isspace(ch) == 0;
          }));
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return std::isspace(ch) == 0; })
              .base(),
          s.end());
}

class OsmiumWindow : public Gtk::Window {
public:
  OsmiumWindow() {
    set_title("Osmium");
    set_default_size(800, 600);

    auto css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(GLOBAL_STYLE.data());
    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(), css_provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    set_child(m_page_scrolled_window);

    m_page_layout.set_orientation(Gtk::Orientation::VERTICAL);
    m_page_layout.set_halign(Gtk::Align::START);
    m_page_layout.set_expand(true);
    m_page_layout.set_margin(10);
    m_page_scrolled_window.set_child(m_page_layout);

    navigate("http://example.org/");

    present();
  }

private:
  std::shared_ptr<Node> m_dom;
  Gtk::Box m_page_layout;
  Gtk::ScrolledWindow m_page_scrolled_window;
  std::string m_current_url;
  std::vector<Gtk::Widget *> m_page_widgets;

  void navigate(const std::string &url) {
    m_current_url = url;
    clear_page();

    std::smatch match;
    assert(std::regex_match(m_current_url, match,
                            std::regex(R"(https?:\/\/([^\/]+)(\/.*)?)")));

    httplib::Client http(match[1].str());
    auto resp = http.Get(match[2].str());

    assert(resp->status == 200);

    m_dom = parse(resp->body);

    render(m_dom, nullptr);
  }

  void render(const std::shared_ptr<Node> &node,
              const std::shared_ptr<Element> &parent) {
    if (node->is_element()) {
      render_element(std::static_pointer_cast<Element>(node), parent);
    } else {
      render_textnode(std::static_pointer_cast<TextNode>(node), parent);
    }
  }

  void render_element(const std::shared_ptr<Element> &el,
                      const std::shared_ptr<Element> & /*parent*/) {
    for (const auto &child : el->children()) {
      render(child, el);
    }
  }

  void render_textnode(const std::shared_ptr<TextNode> &textnode,
                       const std::shared_ptr<Element> &parent) {
    if (parent != nullptr &&
        (parent->name() == "script" || parent->name() == "style" ||
         parent->name() == "title")) {
      return;
    }

    auto content = textnode->content();
    trim(content);
    if (content.empty()) {
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

  void append_widget(Gtk::Widget *widget) {
    m_page_layout.append(*widget);
    m_page_widgets.push_back(widget);
  }

  void clear_page() {
    for (const auto &widget : m_page_widgets) {
      m_page_layout.remove(*widget);
    }
    m_page_widgets.clear();
  }
};

int main(int argc, char *argv[]) {
  setenv("GTK_THEME", "Adwaita:light", 1);

  auto app = Gtk::Application::create("org.antpiasecki.osmium");
  return app->make_window_and_run<OsmiumWindow>(argc, argv);
}