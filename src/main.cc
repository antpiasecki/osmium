#include "window.hh"
#include <gtkmm.h>

int main(int argc, char *argv[]) {
  setenv("GTK_THEME", "Adwaita:light", 1);

  auto app = Gtk::Application::create("org.antpiasecki.osmium");
  return app->make_window_and_run<OsmiumWindow>(argc, argv);
}