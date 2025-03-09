#include "mainwindow.hh"
#include "src/dominspector.hh"
#include "src/net.hh"
#include <QApplication>
#include <QLabel>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPushButton>
#include <QScrollArea>
#include <QtConcurrent/QtConcurrent>

class Anchor : public QLabel {
public:
  Anchor(const QString &text, QUrl url, QWidget *parent = nullptr)
      : QLabel(text, parent), m_url(std::move(url)), m_parent(parent) {}

protected:
  void mousePressEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      dynamic_cast<MainWindow *>(m_parent)->navigate(m_url.toString());
    } else {
      QLabel::mousePressEvent(event);
    }
  }

private:
  QUrl m_url;
  QWidget *m_parent = nullptr;
};

class Image : public QLabel {
public:
  explicit Image(const QUrl &url, QWidget *parent = nullptr) : QLabel(parent) {
    auto future = QtConcurrent::run(
        [](const QUrl &url) -> QPixmap {
          // TODO: pass m_do_verification
          auto resp = Net::get(url, false);
          if (!resp.ok) {
            // TODO: log errors
            return {};
          }

          QPixmap pixmap;
          pixmap.loadFromData(QByteArray::fromRawData(
              resp.result->body.data(),
              static_cast<int>(resp.result->body.size())));
          return pixmap;
        },
        url);

    watcher.setFuture(future);
    connect(&watcher, &QFutureWatcher<QPixmap>::finished, this, [this]() {
      auto pixmap = watcher.result();
      if (!pixmap.isNull()) {
        setPixmap(pixmap);
        show();
      }
    });
  }

private:
  QFutureWatcher<QPixmap> watcher;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_page_layout(new QVBoxLayout(this)) {
  setWindowTitle("Osmium");

  {
    auto *osmium_menu = menuBar()->addMenu("Osmium");

    auto *inspector_action = new QAction("DOM Inspector", this);
    connect(inspector_action, &QAction::triggered, this,
            [this]() { DOMInspector::open(m_dom); });
    osmium_menu->addAction(inspector_action);
    osmium_menu->addSeparator();

    auto *verify_action = new QAction("Verify sites", this);
    verify_action->setCheckable(true);
    verify_action->setChecked(true);
    connect(verify_action, &QAction::toggled, this, [this, verify_action]() {
      m_do_verification = verify_action->isChecked();
    });
    osmium_menu->addAction(verify_action);
    osmium_menu->addSeparator();

    auto *quit_action = new QAction("Quit", this);
    connect(quit_action, &QAction::triggered, this,
            []() { QApplication::exit(); });
    osmium_menu->addAction(quit_action);
  }

  auto *central_widget = new QWidget(this);
  setCentralWidget(central_widget);

  auto *main_layout = new QVBoxLayout(this);
  central_widget->setLayout(main_layout);

  {
    auto *navbar_layout = new QHBoxLayout(this);
    main_layout->addLayout(navbar_layout);

    m_url_bar = new QLineEdit(this);
    connect(m_url_bar, &QLineEdit::returnPressed, this,
            [this]() { this->navigate(m_url_bar->text()); });
    navbar_layout->addWidget(m_url_bar);

    auto *go_button = new QPushButton("→", this);
    go_button->setMaximumWidth(50);
    connect(go_button, &QPushButton::clicked, this,
            [this]() { this->navigate(m_url_bar->text()); });
    navbar_layout->addWidget(go_button);
  }

  auto *page_scroll_area = new QScrollArea(this);
  page_scroll_area->setFrameShape(QFrame::NoFrame);
  auto *page_widget = new QWidget(this);
  m_page_layout->setAlignment(Qt::AlignTop);
  page_widget->setLayout(m_page_layout);
  page_scroll_area->setWidget(page_widget);
  page_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  page_scroll_area->setWidgetResizable(true);
  main_layout->addWidget(page_scroll_area);

  navigate("http://example.org");
}

// TODO: the arg probably shouldnt be named url
void MainWindow::navigate(const QString &url) {
  auto start_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch());

  auto parsed_url = Net::resolve_url(url, m_current_url);
  if (parsed_url.isEmpty()) {
    log("Failed to parse URL: " + url);
    return;
  }

  m_current_url = url;
  m_url_bar->setText(m_current_url);

  // clear the previous page
  // TODO: sometimes very slow?
  clear_page();
  m_page_layout->update();
  QCoreApplication::processEvents();

  log("Navigating to " + url + "...");

  auto resp = Net::get(url, m_do_verification);
  if (!resp.ok) {
    log(resp.error);
    return;
  }
  m_current_url = resp.url.toString();
  m_url_bar->setText(m_current_url);

  log("Parsing " + QString::number(resp.result->body.length() / 1024) +
      " KBs...");
  m_dom = parse(resp.result->body);

  log("Rendering...");
  render(m_dom, nullptr);
  m_page_layout->update();

  auto end_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch());

  log("Rendered " + QString::number(m_page_widgets.size()) + " widgets.");
  log("Done in " + QString::number((end_time - start_time).count()) + " ms.");
}

void MainWindow::render_element(const ElementPtr &el,
                                const ElementPtr & /*parent*/) {
  if (el->name() == "br") {
    auto *label = new QLabel("", this);
    append_widget(label);
  } else if (el->name() == "img") {
    auto *image = new Image(
        Net::resolve_url(QString::fromStdString(el->attributes()["src"]),
                         m_current_url),
        this);
    append_widget(image);
  } else {
    for (const auto &child : el->children()) {
      render(child, el);
    }
  }
}

void MainWindow::render_textnode(const TextNodePtr &textnode,
                                 const ElementPtr &parent) {
  if (parent != nullptr &&
      std::count(s_hidden_elements.begin(), s_hidden_elements.end(),
                 parent->name()) > 0) {
    return;
  }

  auto content =
      QString::fromStdString(textnode->content()).trimmed().replace("\n", " ");
  if (content.isEmpty()) {
    return;
  }

  if (parent != nullptr && parent->name() == "title") {
    m_page_title = content;
    setWindowTitle(m_page_title + " - Osmium");
    return;
  }

  if (parent != nullptr && parent->name() == "a") {
    auto *label = new Anchor(
        content,
        Net::resolve_url(QString::fromStdString(parent->attributes()["href"]),
                         m_current_url),
        this);
    label->setStyleSheet("QLabel { color: #155ca2; }");
    append_widget(label);
    return;
  }

  auto *label = new QLabel(content, this);
  QFont font;

  if (parent != nullptr && (parent->is_heading() || parent->name() == "big")) {
    font.setBold(true);
    label->setContentsMargins(0, 11, 0, 11);

    if (parent->name() == "h1" || parent->name() == "big") {
      font.setPixelSize(32);
    } else if (parent->name() == "h2") {
      font.setPixelSize(24);
    } else if (parent->name() == "h3") {
      font.setPixelSize(18);
    } else if (parent->name() == "h4") {
      font.setPixelSize(16);
    } else if (parent->name() == "h5") {
      font.setPixelSize(13);
    } else if (parent->name() == "h6") {
      font.setPixelSize(11);
    }
  }
  if (parent != nullptr && parent->name() == "p") {
    label->setContentsMargins(0, 16, 0, 16);
  }

  label->setFont(font);
  append_widget(label);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_F12) {
    DOMInspector::open(m_dom);
  }
  QMainWindow::keyPressEvent(event);
}