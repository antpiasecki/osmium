#pragma once
#include <QApplication>
#include <QLineEdit>
#include <QMainWindow>
#include <QVBoxLayout>
#include <httplib.h>
#include <osmium-html/parser.hh>

class MainWindow : public QMainWindow {
public:
  explicit MainWindow(QWidget *parent = nullptr);

  void navigate(QString url);

private:
  // TODO
  static constexpr const char *s_user_agent =
      "Mozilla/5.0 (Linux x86_64) osmium-html/0.1 Osmium/0.1";

  NodePtr m_dom;
  QString m_current_url;
  QString m_page_title;

  QLineEdit *m_url_bar;
  QVBoxLayout *m_page_layout;
  QList<QWidget *> m_page_widgets;

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

  static void log(const QString &s) {
    std::cout << s.toStdString() << std::endl;
  }
};