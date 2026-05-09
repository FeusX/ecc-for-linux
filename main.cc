#include <gtk/gtk.h>

#include "components/keyboard/keyboard.hh"
#include "components/ui/ui.hh"
#include "components/ui/locale.hh"

static KeyboardController kbd;

static void on_activate(GtkApplication *app, gpointer u_data)
{
  ExcaliburGUI* g = new ExcaliburGUI(app, kbd, &TR);
  g->build();
}

int main(int argc, char **argv)
{
  if(!kbd.init())
  { return -1; }

  GtkApplication *app = gtk_application_new("com.feusx.excalibur", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;  
}
