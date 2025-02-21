#include <cassert>
#include <fstream>
#include <gtkmm.h>
#include <memory>
#include <osmium-html/parser.hh>

// stolen from stackoverflow. i love c++
inline std::string trim(const std::string &in) {
  std::string s = in;
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return std::isspace(ch) == 0;
          }));
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return std::isspace(ch) == 0; })
              .base(),
          s.end());
  return s;
}

class OsmiumWindow : public Gtk::Window {
public:
  OsmiumWindow() {
    set_title("Osmium");
    set_default_size(800, 600);

    m_page_layout.set_orientation(Gtk::Orientation::VERTICAL);
    m_page_layout.set_halign(Gtk::Align::START);
    m_page_layout.set_expand(true);
    m_page_layout.set_margin(10);
    set_child(m_page_layout);

    std::ifstream file("test.html");
    assert(file.good());
    std::stringstream ss;
    ss << file.rdbuf();
    m_dom = parse(ss.str());

    render(m_dom, nullptr);

    present();
  }

private:
  std::shared_ptr<Node> m_dom;
  Gtk::Box m_page_layout;
  std::vector<Gtk::Widget *> m_page_widgets;

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
    // don't display scripts and styles
    if (parent != nullptr &&
        (parent->name() == "script" || parent->name() == "style")) {
      return;
    }

    auto content = trim(textnode->content());
    if (content.empty()) {
      return;
    }

    auto *label = Gtk::make_managed<Gtk::Label>(content);
    label->set_wrap(true);
    label->set_xalign(0.0);
    append_widget(label);
  }

  void append_widget(Gtk::Widget *widget) {
    m_page_layout.append(*widget);
    m_page_widgets.push_back(widget);
  }
};

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create("org.antpiasecki.osmium");
  return app->make_window_and_run<OsmiumWindow>(argc, argv);
}