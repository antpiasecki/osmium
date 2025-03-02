#pragma once

#include <gtkmm.h>
#include <httplib.h>
#include <osmium-html/parser.hh>

class OsmiumWindow : public Gtk::Window {
public:
  OsmiumWindow();

private:
  // TODO
  static constexpr const char *s_user_agent =
      "Mozilla/5.0 (Linux x86_64) osmium-html/0.1 Osmium/0.1";

  NodePtr m_dom;
  std::string m_current_url;
  std::string m_page_title;

  Gtk::Box m_main_layout;
  Gtk::Box m_navbar_layout;
  Gtk::Entry m_url_entry;
  Gtk::Button m_go_button{"→"};
  Gtk::Box m_page_layout;
  Gtk::ScrolledWindow m_page_scrolled_window;
  std::vector<Gtk::Widget *> m_page_widgets;

  void navigate(std::string url);

  void render(const NodePtr &node, const ElementPtr &parent) {
    if (node->is_element()) {
      render_element(std::static_pointer_cast<Element>(node), parent);
    } else {
      render_textnode(std::static_pointer_cast<TextNode>(node), parent);
    }
  }

  void render_element(const ElementPtr &el, const ElementPtr &parent);

  void render_textnode(const TextNodePtr &textnode, const ElementPtr &parent);

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

  static void log(const std::string &s);
};