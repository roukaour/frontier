#include <cstdlib>

#pragma warning(push, 0)
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Box.H>
#pragma warning(pop)

#include "os-font.h"
#include "icons.h"
#include "widgets.h"
#include "main-window.h"
#include "toolbar.h"

Toolbar::Toolbar(int ww, int /*wh*/) : Fl_Toolbar(0, 23, ww, 28, NULL) {
	// Populate toolbar
	_open_tb = new Fl_Toolbar_Button(0, 0, 56, 24);
	_save_tb = new Fl_Toolbar_Button(0, 0, 51, 24);
	new Fl_Spacer(0, 0, 2, 24);
	_decimate_tb = new Fl_Toolbar_Button(0, 0, 77, 24);
	_expand_tb = new Fl_Toolbar_Button(0, 0, 65, 24);
	_interpolate_tb = new Fl_Toolbar_Button(0, 0, 83, 24);
	_erode_tb = new Fl_Toolbar_Button(0, 0, 57, 24);
	new Fl_Spacer(0, 0, 2, 24);
	_zoom_in_tb = new Fl_Toolbar_Button(0, 0, 71, 24);
	_zoom_out_tb = new Fl_Toolbar_Button(0, 0, 80, 24);
	new Fl_Spacer(0, 0, 2, 24);
	new Fl_Text(0, 0, 90, 24, "Color Scheme:");
	_color_scheme_choice = new Fl_Choice(0, 0, 100, 24);
	new Fl_Spacer(0, 0, 2, 24);
	_render_3d_tb = new Fl_Toolbar_Toggle_Button(0, 0, 80, 24);
	_normals_tb = new Fl_Toolbar_Button(0, 0, 71, 24);
	new Fl_Text(0, 0, 35, 24, "Scale:");
	_scale_slider = new Fl_Default_Slider(0, 0, 100, 24);
	// Initialize toolbar
	spacing(0);
	clip_children(1);
	// Initialize toolbar widgets
	Fl_Tooltip::font(OS_FONT);
	Fl_Tooltip::size(OS_FONT_SIZE);
	Fl_Tooltip::delay(0.5f);
	Main_Window *mw = static_cast<Main_Window *>(parent());
	_open_tb->tooltip("Open...");
	_open_tb->label("Open");
	_open_tb->image(OPEN_ICON);
	_open_tb->callback((Fl_Callback *)Main_Window::open_cb, mw);
	_open_tb->take_focus();
	_save_tb->tooltip("Save...");
	_save_tb->label("Save");
	_save_tb->image(SAVE_ICON);
	_save_tb->callback((Fl_Callback *)Main_Window::save_cb, mw);
	_decimate_tb->tooltip("Decimate...");
	_decimate_tb->label("Decimate");
	_decimate_tb->image(DECIMATE_ICON);
	_decimate_tb->callback((Fl_Callback *)Main_Window::decimate_cb, mw);
	_expand_tb->tooltip("Expand...");
	_expand_tb->label("Expand");
	_expand_tb->image(EXPAND_ICON);
	_expand_tb->callback((Fl_Callback *)Main_Window::expand_cb, mw);
	_interpolate_tb->tooltip("Interpolate...");
	_interpolate_tb->label("Interpolate");
	_interpolate_tb->image(INTERPOLATE_ICON);
	_interpolate_tb->callback((Fl_Callback *)Main_Window::interpolate_cb, mw);
	_erode_tb->tooltip("Erode...");
	_erode_tb->label("Erode");
	_erode_tb->image(ERODE_ICON);
	_erode_tb->callback((Fl_Callback *)Main_Window::erode_cb, mw);
	_zoom_in_tb->tooltip("Zoom In");
	_zoom_in_tb->label("Zoom In");
	_zoom_in_tb->image(ZOOM_IN_ICON);
	_zoom_in_tb->callback((Fl_Callback *)Main_Window::zoom_in_cb, mw);
	_zoom_out_tb->tooltip("Zoom Out");
	_zoom_out_tb->label("Zoom Out");
	_zoom_out_tb->image(ZOOM_OUT_ICON);
	_zoom_out_tb->callback((Fl_Callback *)Main_Window::zoom_out_cb, mw);
	_color_scheme_choice->labelfont(OS_FONT);
	_color_scheme_choice->labelsize(OS_FONT_SIZE);
	_color_scheme_choice->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_color_scheme_choice->textfont(OS_FONT);
	_color_scheme_choice->textsize(OS_FONT_SIZE);
	_color_scheme_choice->add("Grayscale", 0, NULL, mw, 0);
	_color_scheme_choice->add("Elevation (Red)", 0, NULL, mw, 0);
	_color_scheme_choice->add("Hardness (Green)", 0, NULL, mw, 0);
	_color_scheme_choice->add("Solubility (Blue)", 0, NULL, mw, 0);
	_color_scheme_choice->add("Combination (White)", 0, NULL, mw, 0);
	_color_scheme_choice->add("Artificial (Earth)", 0, NULL, mw, 0);
	_color_scheme_choice->value(Color_Scheme::GRAYSCALE);
	_color_scheme_choice->callback((Fl_Callback *)Main_Window::color_scheme_tb_cb, mw);
	_render_3d_tb->tooltip("Render 3D");
	_render_3d_tb->label("Render 3D");
	_render_3d_tb->image(RENDER_3D_ICON);
	_render_3d_tb->callback((Fl_Callback *)Main_Window::render_3d_tb_cb, mw);
	_normals_tb->tooltip("Normals");
	_normals_tb->label("Normals");
	_normals_tb->image(NORMALS_ICON);
	_normals_tb->callback((Fl_Callback *)Main_Window::normals_cb, mw);
	_scale_slider->bounds(1.0, 1001.0);
	_scale_slider->step(10.0);
	_scale_slider->default_value(101.0);
	_scale_slider->callback((Fl_Callback *)Main_Window::scale_cb, mw);
	end();
}

float Toolbar::toolbar_scale() const {
	return (float)_scale_slider->value();
}
