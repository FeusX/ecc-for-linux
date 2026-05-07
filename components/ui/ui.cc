#include "ui.hh"

ExcaliburGUI::ExcaliburGUI(GtkApplication* app, KeyboardController& kbd) : app_(app), kbd_(kbd) {}

void ExcaliburGUI::build()
{
  window_ = gtk_application_window_new(app_);

  gtk_window_set_title(GTK_WINDOW(window_), "Excalibur Control Center Linux");

  gtk_window_set_default_size(GTK_WINDOW(window_), 1280, 960);

  gtk_window_set_resizable(GTK_WINDOW(window_), FALSE);

  gtk_window_present(GTK_WINDOW(window_));
}
