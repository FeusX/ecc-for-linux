#include "ui.hh"
#include "locale.hh"
#include <cmath>
#include <fstream>
#include <pwd.h>
#include <unistd.h>

ExcaliburGUI::ExcaliburGUI(GtkApplication* app, KeyboardController& kbd, const Locale* locale, OptimusSwitch& gpu_switch) 
    : app_(app), kbd_(kbd), lang(locale), switch_(gpu_switch) {}

GtkWidget* create_ui_button(const char* label, const char* css_class = nullptr)
{
  GtkWidget* btn = gtk_button_new_with_label(label);
  if(css_class) { gtk_widget_add_css_class(btn, css_class); }
  return btn;
}

void ExcaliburGUI::build()
{
  window_ = gtk_application_window_new(app_);
  gtk_window_set_title(GTK_WINDOW(window_), "Excalibur Control Center Linux");
  gtk_window_set_default_size(GTK_WINDOW(window_), 1280, 960);
  gtk_window_set_resizable(GTK_WINDOW(window_), FALSE);

  GtkWidget* o = gtk_overlay_new();
  gtk_window_set_child(GTK_WINDOW(window_), o);

  drawing_area_ = gtk_drawing_area_new();
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area_), on_draw, this, NULL);
  gtk_overlay_set_child(GTK_OVERLAY(o), drawing_area_);

  // --- ZONE SELECTION ---
  GtkWidget* top_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_valign(top_box, GTK_ALIGN_START);
  gtk_widget_set_halign(top_box, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top(top_box, 20);

  const char* zone_labels[] = { lang->zone_left.c_str(), lang->zone_mid.c_str(), lang->zone_right.c_str(), lang->zone_all.c_str() };
  ZoneID zone_ids[] = { ZoneID::LEFT, ZoneID::MID, ZoneID::RIGHT, ZoneID::ALL };
  
  GtkWidget* first_zone = nullptr;
  for(int i = 0; i < 4; i++)
  {
    GtkWidget* b = gtk_toggle_button_new_with_label(zone_labels[i]);
    if(i == 0) { first_zone = b; }
    else { gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(b), GTK_TOGGLE_BUTTON(first_zone)); }

    g_signal_connect(b, "toggled", G_CALLBACK(+[](GtkToggleButton* btn, gpointer data) {
      if (gtk_toggle_button_get_active(btn)) {
        auto* self = static_cast<ExcaliburGUI*>(data);
        self->selected_zone = (ZoneID)GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "id"));
      }
    }), this);

    g_object_set_data(G_OBJECT(b), "id", GINT_TO_POINTER(zone_ids[i]));
    gtk_box_append(GTK_BOX(top_box), b);
  }
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(first_zone), TRUE);
  gtk_overlay_add_overlay(GTK_OVERLAY(o), top_box);

  // --- GPU SWITCH ---
  GtkWidget* gpu_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
  gtk_widget_set_valign(gpu_box, GTK_ALIGN_START);
  gtk_widget_set_halign(gpu_box, GTK_ALIGN_CENTER);
  gtk_widget_set_margin_top(gpu_box, 65);

  GtkWidget* nvidia_btn = gtk_toggle_button_new_with_label("Nvidia");
  GtkWidget* intel_btn  = gtk_toggle_button_new_with_label("Intel");
  gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(intel_btn), GTK_TOGGLE_BUTTON(nvidia_btn));

  g_signal_connect(nvidia_btn, "toggled", G_CALLBACK(+[](GtkToggleButton* btn, gpointer data) {
    if(gtk_toggle_button_get_active(btn)) {
      auto* self = static_cast<ExcaliburGUI*>(data);
      GtkAlertDialog *dialog = gtk_alert_dialog_new("!!!!!!!!");
      gtk_alert_dialog_set_detail(dialog, self->lang->gpu_warning.c_str());
      gtk_alert_dialog_show(dialog, GTK_WINDOW(self->window_));

      while(g_main_context_pending(NULL)) { g_main_context_iteration(NULL, FALSE); }
      self->switch_.switch_gpus(1);
    }
  }), this);

  g_signal_connect(intel_btn, "toggled", G_CALLBACK(+[](GtkToggleButton* btn, gpointer data) {
    if(gtk_toggle_button_get_active(btn)) {
      auto* self = static_cast<ExcaliburGUI*>(data);

      GtkAlertDialog *dialog = gtk_alert_dialog_new("!!!!!!!!");
      gtk_alert_dialog_set_detail(dialog, self->lang->gpu_warning.c_str());
      gtk_alert_dialog_show(dialog, GTK_WINDOW(self->window_));
      
      while(g_main_context_pending(NULL)) { g_main_context_iteration(NULL, FALSE); }
      
      self->switch_.switch_gpus(0);
    }
  }), this);

  gtk_box_append(GTK_BOX(gpu_box), nvidia_btn);
  gtk_box_append(GTK_BOX(gpu_box), intel_btn);
  gtk_overlay_add_overlay(GTK_OVERLAY(o), gpu_box);

  // --- COLOR PATTERNS ---
  GtkWidget* right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_valign(right_box, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(right_box, GTK_ALIGN_END);
  gtk_widget_set_margin_end(right_box, 20);

  const char* pattern_labels[] = { lang->pattern_solid.c_str(), lang->pattern_pulse.c_str(), lang->pattern_cycle.c_str(), lang->pattern_ambilight.c_str() };
  KBPattern patterns[] = { KBPattern::SOLID, KBPattern::BREATHE, KBPattern::CYCLE, KBPattern::AMBILIGHT };

  GtkWidget* first_pattern = nullptr;
  for(int i = 0; i < 4; i++) // DEAR SOFTWARE DEVELOPERS, DO NOT HARD CODE VALUES LIKE THIS.
  {
    GtkWidget* b = gtk_toggle_button_new_with_label(pattern_labels[i]);
    if(i == 0) { first_pattern = b; }
    else { gtk_toggle_button_set_group(GTK_TOGGLE_BUTTON(b), GTK_TOGGLE_BUTTON(first_pattern)); }

    g_signal_connect(b, "toggled", G_CALLBACK(+[](GtkToggleButton* btn, gpointer data) {
      if(gtk_toggle_button_get_active(btn))
      {
        auto* self = static_cast<ExcaliburGUI*>(data);
        int raw_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(btn), "id"));
        self->selected_pattern = static_cast<KBPattern>(raw_id);
        g_print("Pattern selected: %d\n", raw_id);
        self->kbd_.set_mode(self->selected_pattern);
      }
    }), this);

    g_object_set_data(G_OBJECT(b), "id", GINT_TO_POINTER(static_cast<int>(patterns[i])));
    gtk_box_append(GTK_BOX(right_box), b);
  }
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(first_pattern), TRUE);
  gtk_overlay_add_overlay(GTK_OVERLAY(o), right_box);

  // --- COLOR PICKER ---
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  GtkWidget* color_chooser = gtk_color_chooser_widget_new();
  gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(color_chooser), FALSE);
  g_object_set(color_chooser, "show-editor", TRUE, NULL);

  gtk_widget_set_halign(color_chooser, GTK_ALIGN_START);
  gtk_widget_set_valign(color_chooser, GTK_ALIGN_END);
  gtk_widget_set_margin_start(color_chooser, 20);
  gtk_widget_set_margin_bottom(color_chooser, 20);
    
  g_signal_connect(color_chooser, "notify::rgba", G_CALLBACK(+[](GObject* obj, GParamSpec*, gpointer data) {
    auto* self = static_cast<ExcaliburGUI*>(data);
    GdkRGBA rgba;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(obj), &rgba);
#pragma GCC diagnostic pop
    self->curr_r = rgba.red;
    self->curr_g = rgba.green;
    self->curr_b = rgba.blue;
  }), this);
#pragma GCC diagnostic pop
  
  gtk_overlay_add_overlay(GTK_OVERLAY(o), color_chooser);

  // --- APPLY BUTTON ---
  GtkWidget* apply_btn = create_ui_button(lang->btn_apply.c_str(), "suggested-action");
  gtk_widget_set_halign(apply_btn, GTK_ALIGN_END);
  gtk_widget_set_valign(apply_btn, GTK_ALIGN_END);
  gtk_widget_set_margin_end(apply_btn, 20);
  gtk_widget_set_margin_bottom(apply_btn, 20);

  g_object_set_data(G_OBJECT(apply_btn), "btn_solid", first_pattern);

  g_signal_connect(apply_btn, "clicked", G_CALLBACK(+[](GtkButton* btn, gpointer data) {
    auto* self = static_cast<ExcaliburGUI*>(data);
    uint8_t r = (uint8_t)(self->curr_r * 255);
    uint8_t g = (uint8_t)(self->curr_g * 255);
    uint8_t b = (uint8_t)(self->curr_b * 255);
        
    self->kbd_.set_color(r, g, b, 0xFF, self->selected_zone);

    GtkWidget* btn_solid = GTK_WIDGET(g_object_get_data(G_OBJECT(btn), "btn_solid"));
    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn_solid)))
    { gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(btn_solid), TRUE); }
    
    gtk_widget_queue_draw(self->drawing_area_);
  }), this);
  gtk_overlay_add_overlay(GTK_OVERLAY(o), apply_btn);

  // --- LANGUAGE SWITCH ---
  GtkWidget* lang_toggle = gtk_button_new_with_label("TR / EN");
  gtk_button_set_has_frame(GTK_BUTTON(lang_toggle), FALSE);
  gtk_widget_set_halign(lang_toggle, GTK_ALIGN_START);
  gtk_widget_set_valign(lang_toggle, GTK_ALIGN_START);
  gtk_widget_set_margin_start(lang_toggle, 15);
  gtk_widget_set_margin_top(lang_toggle, 15);

  g_signal_connect(lang_toggle, "clicked", G_CALLBACK(+[](GtkButton*, gpointer data) {
    auto* self = static_cast<ExcaliburGUI*>(data);
    if(access(get_lang_path().c_str(), F_OK) == 0)
    { set_lang_en(); }
    else { set_lang_tr(); }
    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(self->window_),
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_INFO,
                                                 GTK_BUTTONS_OK,
                                                 "Language changed. Please restart the app.");
      
      g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_window_destroy), dialog);
      gtk_window_present(GTK_WINDOW(dialog));
  }), this);

  gtk_overlay_add_overlay(GTK_OVERLAY(o), lang_toggle);

  gtk_window_present(GTK_WINDOW(window_));
}

void ExcaliburGUI::on_draw(GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer data)
{
  ExcaliburGUI* self = static_cast<ExcaliburGUI*>(data);
  float cx = width / 2.0f;
  float cy = height / 2.0f;

  // fill the background
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_paint(cr);

  float back_wall_scale = 0.35f; 
  float bw_w = width * back_wall_scale;
  float bw_h = height * back_wall_scale;
  float bw_left = cx - bw_w / 2;
  float bw_right = cx + bw_w / 2;
  float bw_top = cy - bw_h / 2;
  float bw_bottom = cy + bw_h / 2;

  auto draw_geo = [&]()
  {
    int divs = 10;
    for(int i = 0; i <= divs; i++)
    {
      float x_ratio = (float)i / divs;
      float x_outer = width * x_ratio;
      float x_inner = bw_left + (bw_w * x_ratio);
        
      cairo_move_to(cr, x_outer, 0);
      cairo_line_to(cr, x_inner, bw_top);
      cairo_move_to(cr, x_outer, height);
      cairo_line_to(cr, x_inner, bw_bottom);

      float y_ratio = (float)i / divs;
      float y_outer = height * y_ratio;
      float y_inner = bw_top + (bw_h * y_ratio);

      cairo_move_to(cr, 0, y_outer);
      cairo_line_to(cr, bw_left, y_inner);
      cairo_move_to(cr, width, y_outer);
      cairo_line_to(cr, bw_right, y_inner);
    }

    for(int i = 1; i <= 6; i++)
    {
      float t = (float)i / 6.0f;
      float w = width * (1.0f - t) + bw_w * t;
      float h = height * (1.0f - t) + bw_h * t;
      cairo_rectangle(cr, cx - w/2, cy - h/2, w, h);
    }

    for(int i = 1; i < divs; i++)
    {
      float x = bw_left + (bw_w / divs) * i;
      cairo_move_to(cr, x, bw_top);
      cairo_line_to(cr, x, bw_bottom);
    
      float y = bw_top + (bw_h / divs) * i;
      cairo_move_to(cr, bw_left, y);
      cairo_line_to(cr, bw_right, y);
    }

    cairo_stroke(cr); 
  };

  // glow layer helper function
  auto draw_glow_layer = [&](float opacity, float line_width, float offset_y) {
    cairo_save(cr);
    cairo_translate(cr, 0, offset_y);
    cairo_set_source_rgba(cr, self->curr_r, self->curr_g, self->curr_b, opacity);
    cairo_set_line_width(cr, line_width);
    draw_geo();
    cairo_restore(cr);
  };

  cairo_set_source_rgba(cr, self->curr_r * 0.2, self->curr_g * 0.2, self->curr_b * 0.2, 1.0);
  cairo_set_line_width(cr, 15.0);
  draw_geo();

  // astigmatism effect
  draw_glow_layer(0.06, 12.0, -2.0);
  draw_glow_layer(0.06, 12.0,  2.0);
  draw_glow_layer(0.09, 8.0,   0.0);

  cairo_set_source_rgba(cr, self->curr_r, self->curr_g, self->curr_b, 1.0);
  cairo_set_line_width(cr, 1.3);
  draw_geo();

  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.4);
  cairo_set_line_width(cr, 0.6);
  draw_geo();
}

std::string get_lang_path()
{
  const char* sudo_user = std::getenv("SUDO_USER");
  std::string path;

  if(sudo_user)
  {
    struct passwd* pw = getpwnam(sudo_user);
    if(pw) path = pw->pw_dir;
  }
  else
  {
    const char* home = std::getenv("HOME");
    if(home) path = home;
  }
    
  if(path.empty()) return "/tmp/.ecc_lang";
  return path + "/.cache/.ecc_lang";
}

void set_lang_en() { std::remove(get_lang_path().c_str()); }

void set_lang_tr() { std::ofstream file(get_lang_path()); file.close();}
