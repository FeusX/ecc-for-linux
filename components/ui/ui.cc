#include "ui.hh"
#include <cmath>

ExcaliburGUI::ExcaliburGUI(GtkApplication* app, KeyboardController& kbd) : app_(app), kbd_(kbd) {}

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

  GtkWidget* ui_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_valign(ui_container, GTK_ALIGN_END);
  gtk_widget_set_margin_bottom(ui_container, 30);
  gtk_widget_set_margin_start(ui_container, 20);
  gtk_widget_set_margin_end(ui_container, 20);

  GtkColorDialog* dialog = gtk_color_dialog_new();
  GtkWidget* color_btn = gtk_color_dialog_button_new(dialog);

  GtkWidget* save_btn = gtk_button_new_with_label("SYNC TO HARDWARE");
  gtk_widget_add_css_class(save_btn, "suggested-action");

  gtk_box_append(GTK_BOX(ui_container), color_btn);
  gtk_box_append(GTK_BOX(ui_container), save_btn);

  gtk_overlay_add_overlay(GTK_OVERLAY(o), ui_container);

  g_signal_connect(color_btn, "notify::rgba", G_CALLBACK(+[](GObject* obj, GParamSpec* pspec, gpointer data) {
        ExcaliburGUI* self = static_cast<ExcaliburGUI*>(data);
        const GdkRGBA *rgba = gtk_color_dialog_button_get_rgba(GTK_COLOR_DIALOG_BUTTON(obj));

        if(rgba)
        {
          self->curr_r = rgba->red;
          self->curr_g = rgba->green;
          self->curr_b = rgba->blue;
        }

        gtk_widget_queue_draw(self->drawing_area_);
    }), this);

  gtk_window_present(GTK_WINDOW(window_));
}

void ExcaliburGUI::on_draw(GtkDrawingArea* area, cairo_t* cr, int width, int height, gpointer data)
{
  ExcaliburGUI* self = static_cast<ExcaliburGUI*>(data);
  float cx = width / 2.0f;
  float cy = height / 2.0f;

  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
  cairo_paint(cr);

  cairo_set_source_rgba(cr, self->curr_r, self->curr_g, self->curr_b, 0.6);
  cairo_set_line_width(cr, 1.2);

  float back_wall_scale = 0.35f; 
  float bw_w = width * back_wall_scale;
  float bw_h = height * back_wall_scale;
  float bw_left = cx - bw_w/2;
  float bw_right = cx + bw_w/2;
  float bw_top = cy - bw_h/2;
  float bw_bottom = cy + bw_h/2;

  for(int i = 0; i < 6; i++)
  {
    float t = i / 5.0f;
    float w = width * (1.0f - t) + bw_w * t;
    float h = height * (1.0f - t) + bw_h * t;
    cairo_rectangle(cr, cx - w/2, cy - h/2, w, h);
  }

  cairo_stroke(cr);

  int divs = 10;
  for(int i = 0; i <= divs; i++)
  {
    float x_ratio = (float)i / divs;
    float y_ratio = (float)i / divs;

    float x_outer = width * x_ratio;
    float x_inner = bw_left + (bw_w * x_ratio);
        
    cairo_move_to(cr, x_outer, 0);      cairo_line_to(cr, x_inner, bw_top);
    cairo_move_to(cr, x_outer, height); cairo_line_to(cr, x_inner, bw_bottom);

    float y_outer = height * y_ratio;
    float y_inner = bw_top + (bw_h * y_ratio);

    cairo_move_to(cr, 0, y_outer);     cairo_line_to(cr, bw_left, y_inner);
    cairo_move_to(cr, width, y_outer);  cairo_line_to(cr, bw_right, y_inner);
  }
  
  cairo_stroke(cr);

  for(int i = 1; i < divs; i++)
  {
    float x = bw_left + (bw_w / divs) * i;
    cairo_move_to(cr, x, bw_top); cairo_line_to(cr, x, bw_bottom);
    
    float y = bw_top + (bw_h / divs) * i;
    cairo_move_to(cr, bw_left, y); cairo_line_to(cr, bw_right, y);
  }

  cairo_stroke(cr);
}
