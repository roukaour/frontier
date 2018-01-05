#ifndef MENU_BAR_H
#define MENU_BAR_H

#include <cstdlib>

#pragma warning(push, 0)
#include <FL/Fl_Menu_Bar.H>
#pragma warning(pop)

#include "os-font.h"
#include "draw-state.h"
#include "widgets.h"

class Menu_Bar : public Fl_Menu_Bar {
private:
	Fl_Menu_Item *_render_3d, *_grayscale_mi, *_elevation_red_mi, *_hardness_green_mi, *_solubility_blue_mi,
		*_combination_white_mi, *_artificial_earth_mi;
public:
	Menu_Bar(int ww, int wh);
	inline bool menu_bar_render_3d(void) const { return _render_3d->value() != 0; }
	inline void menu_bar_render_3d(bool r) { if (r) { _render_3d->set(); } else { _render_3d->clear(); } }
	Color_Scheme menu_bar_color_scheme(void) const;
	void menu_bar_color_scheme(Color_Scheme cs);
};

#endif
