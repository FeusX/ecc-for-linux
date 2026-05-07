#ifndef UI_HH
#define UI_HH

#include <gtk/gtk.h>
#include "../keyboard/keyboard.hh"

class ExcaliburGUI {
public:
  ExcaliburGUI(GtkApplication* app, KeyboardController &kbd);
  void build();
  
private:
  GtkApplication *app_;
  KeyboardController& kbd_;
  GtkWidget *window_;
};

#endif
