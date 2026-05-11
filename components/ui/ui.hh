#ifndef UI_HH
#define UI_HH

#include <gtk/gtk.h>
#include <string>
#include "../keyboard/keyboard.hh"
#include "locale.hh"

class ExcaliburGUI {
public:
  ExcaliburGUI(GtkApplication* app, KeyboardController& kbd, const Locale* locale);
  void build(); 
  
private:
  GtkApplication *app_;
  KeyboardController& kbd_;
  GtkWidget *window_;
  GtkWidget *drawing_area_;

  const Locale* lang;

  float curr_r = 0.0, curr_g = 1.0, curr_b = 0.0;

  ZoneID selected_zone = ZoneID::ALL;
  KBPattern selected_pattern = KBPattern::SOLID;

  static void on_draw(GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer data);
};

std::string get_lang_path();  
void set_lang_en();  
void set_lang_tr();

#endif
