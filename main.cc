#include <gtk/gtk.h>
#include <unistd.h>
#include <string>
#include <fstream>

#include "components/keyboard/keyboard.hh"
#include "components/ui/ui.hh"
#include "components/ui/locale.hh"
#include "components/gpu/optimus.hh"

static KeyboardController kbd;
static OptimusSwitch gpu_switch;

static void on_activate(GtkApplication *app, gpointer u_data)
{
  const Locale* selected = static_cast<const Locale*>(u_data);
  
  ExcaliburGUI* g = new ExcaliburGUI(app, kbd, selected, gpu_switch);
  g->build();
}

int main(int argc, char **argv)
{
  if(!kbd.init())
  { return -1; }

  const Locale* selected_locale;
  if(access(get_lang_path().c_str(), F_OK) == 0)
  { selected_locale = &TR; }
  else
  { selected_locale = &EN; }

  GtkApplication *app = gtk_application_new("com.feusx.excalibur", G_APPLICATION_DEFAULT_FLAGS);

  g_signal_connect(app, "activate", G_CALLBACK(on_activate), (gpointer)selected_locale);

  int status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;  
}
