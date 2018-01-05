#pragma once

#include <cstdlib>

#pragma warning(push, 0)
#include <FL/Fl_Choice.H>
#pragma warning(pop)

#include "draw-state.h"
#include "widgets.h"

class Toolbar : public Fl_Toolbar {
private:
	Fl_Toolbar_Button *_open_tb, *_save_tb, *_decimate_tb, *_expand_tb, *_interpolate_tb, *_erode_tb, *_zoom_in_tb,
		*_zoom_out_tb, *_normals_tb;
	Fl_Choice *_color_scheme_choice;
	Fl_Toolbar_Toggle_Button *_render_3d_tb;
	Fl_Default_Slider *_scale_slider;
public:
	Toolbar(int ww, int wh);
	inline bool toolbar_render_3d(void) const { return _render_3d_tb->value() != 0; }
	inline void toolbar_render_3d(bool r) { _render_3d_tb->value(r ? 1 : 0); }
	inline Color_Scheme toolbar_color_scheme(void) const { return (Color_Scheme)_color_scheme_choice->value(); }
	inline void toolbar_color_scheme(Color_Scheme cs) { _color_scheme_choice->value((int)cs); }
	float toolbar_scale(void) const;
};
