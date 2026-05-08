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
  GtkWidget *drawing_area_;

  float curr_r = 0.0, curr_g = 1.0, curr_b = 0.0;

  static void on_draw(GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer data);
};

#endif
