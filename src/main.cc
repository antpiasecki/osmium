#include "mainwindow.hh"
#include <QApplication>

int main(int argc, char *argv[]) {
  setenv("GTK_THEME", "Adwaita:light", 1);

  QApplication app(argc, argv);
  MainWindow window;
  window.resize(1024, 768);
  window.show();
  return QApplication::exec();
}