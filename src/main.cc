#include <cassert>
#include <fstream>
#include <gtkmm.h>
#include <memory>
#include <osmium-html/parser.hh>

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

    render(m_dom);

    present();
  }

private:
  std::shared_ptr<Node> m_dom;
  Gtk::Box m_page_layout;

  void render(const std::shared_ptr<Node> &node) {
    if (node->is_element()) {
      render_element(std::static_pointer_cast<Element>(node));
    } else {
      render_textnode(std::static_pointer_cast<TextNode>(node));
    }
  }

  void render_element(const std::shared_ptr<Element> &el) {
    for (const auto &child : el->children()) {
      render(child);
    }
  }

  void render_textnode(const std::shared_ptr<TextNode> &textnode) {
    std::cout << "rendering " << textnode->content() << std::endl;
  }
};

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create("org.antpiasecki.osmium");
  return app->make_window_and_run<OsmiumWindow>(argc, argv);
}