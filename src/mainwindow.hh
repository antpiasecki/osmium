#pragma once
#include <QLineEdit>
#include <QMainWindow>
#include <QVBoxLayout>
#include <osmium-html/parser.hh>

class MainWindow : public QMainWindow {
public:
  explicit MainWindow(QWidget *parent = nullptr);

  void navigate(const QString &url);

  // TODO: need to find better place for this
  static void log(const QString &s) {
    std::cout << s.toStdString() << std::endl;
  }

protected:
  void keyPressEvent(QKeyEvent *event) override;

private:
  // TODO: what if we're not on linux
  static constexpr const char *s_user_agent =
      "Mozilla/5.0 (Linux x86_64) osmium-html/0.1 Osmium/0.1";

  static constexpr std::array<std::string_view, 9> s_hidden_elements = {
      "script",   "style",  "head", "iframe", "link",
      "template", "option", "meta", "base"};

  bool m_do_verification = true;

  NodePtr m_dom;
  QString m_current_url;
  QString m_page_title;

  QLineEdit *m_url_bar;
  QVBoxLayout *m_page_layout;
  QList<QWidget *> m_page_widgets;

  QUrl resolve_url(const QString &url);

  void render(const NodePtr &node, const ElementPtr &parent) {
    if (node->is_element()) {
      render_element(std::static_pointer_cast<Element>(node), parent);
    } else {
      render_textnode(std::static_pointer_cast<TextNode>(node), parent);
    }
  }

  void render_element(const ElementPtr &el, const ElementPtr &parent);

  void render_textnode(const TextNodePtr &textnode, const ElementPtr &parent);

  void append_widget(QWidget *widget) {
    m_page_layout->addWidget(widget);
    m_page_widgets.append(widget);
  }

  void clear_page() {
    qDeleteAll(m_page_widgets);
    m_page_widgets.clear();
  }
};