#include "dominspector.hh"
#include <QBoxLayout>
#include <memory>

DOMInspector::DOMInspector(const NodePtr &dom) {
  setWindowTitle("DOM Inspector");
  resize(600, 700);

  auto *layout = new QVBoxLayout(this);
  auto *tree = new QTreeView(this);
  layout->addWidget(tree);
  setLayout(layout);

  auto *model = new QStandardItemModel(this);
  for (const auto &el : std::static_pointer_cast<Element>(dom)->children()) {
    render(el, model->invisibleRootItem());
  }
  tree->setModel(model);
  tree->setHeaderHidden(true);
  tree->expandAll();
}

void DOMInspector::render(const NodePtr &node, QStandardItem *parent) {
  if (node->is_element()) {
    auto el = std::static_pointer_cast<Element>(node);

    QString s = "<" + QString::fromStdString(el->name());
    for (const auto &a : el->attributes()) {
      s += QString::fromStdString(" " + escape(a.first) + "=\"" +
                                  escape(a.second) + "\"");
    }
    s += ">";

    auto *item = new QStandardItem(s);
    for (const auto &child : el->children()) {
      render(child, item);
    }
    parent->appendRow(item);
  } else {
    auto textnode = std::static_pointer_cast<TextNode>(node);

    parent->appendRow(new QStandardItem(
        "\"" + QString::fromStdString(escape(textnode->content())) + "\""));
  }
}