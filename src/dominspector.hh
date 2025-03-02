#pragma once
#include <QStandardItem>
#include <QTreeView>
#include <QWindow>
#include <osmium-html/dom.hh>

class DOMInspector : public QWidget {
public:
  explicit DOMInspector(const NodePtr &dom);

  static void open(const NodePtr &dom) {
    auto *inspector = new DOMInspector(dom);
    inspector->show();
  }

private:
  void render(const NodePtr &node, QStandardItem *parent);
};