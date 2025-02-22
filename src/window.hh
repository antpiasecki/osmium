#pragma once

#include <gtkmm.h>
#include <httplib.h>
#include <osmium-html/parser.hh>

class OsmiumWindow : public Gtk::Window {
public:
  OsmiumWindow();

private:
  std::shared_ptr<Node> m_dom;
  std::string m_current_url;

  Gtk::Box m_main_layout;
  Gtk::Box m_navbar_layout;
  Gtk::Entry m_url_entry;
  Gtk::Button m_go_button{"→"};
  Gtk::Box m_page_layout;
  Gtk::ScrolledWindow m_page_scrolled_window;
  std::vector<Gtk::Widget *> m_page_widgets;

  void navigate(const std::string &url);

  void render(const std::shared_ptr<Node> &node,
              const std::shared_ptr<Element> &parent) {
    if (node->is_element()) {
      render_element(std::static_pointer_cast<Element>(node), parent);
    } else {
      render_textnode(std::static_pointer_cast<TextNode>(node), parent);
    }
  }

  void render_element(const std::shared_ptr<Element> &el,
                      const std::shared_ptr<Element> & /*parent*/);

  void render_textnode(const std::shared_ptr<TextNode> &textnode,
                       const std::shared_ptr<Element> &parent);

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