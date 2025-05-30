#include "mainwindow.hh"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  MainWindow window;
  window.resize(1024, 768);
  window.show();

  if (argc > 1) {
    window.navigate(argv[1]);
  } else {
    window.navigate("http://example.org");
  }

  return QApplication::exec();
}