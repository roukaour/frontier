#include <cstdlib>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Menu_Bar.H>
#pragma warning(pop)

#include "os-font.h"
#include "main-window.h"
#include "menu-bar.h"

#define MENU_BAR_STYLE FL_NORMAL_LABEL, OS_FONT, OS_FONT_SIZE, FL_FOREGROUND_COLOR
#define _P "     "

Menu_Bar::Menu_Bar(int ww, int /*wh*/) : Fl_Menu_Bar(0, 0, ww, 23, NULL) {
	// Populate menu bar
	Main_Window *mw = static_cast<Main_Window *>(parent());
	Fl_Menu_Item menu_items[] = {
		// label, shortcut, callback, data, flags, labeltype, font, size, color
		{"&File", 0, NULL, NULL, FL_SUBMENU, MENU_BAR_STYLE},
			{"&New..."_P, FL_COMMAND + 'n', (Fl_Callback *)Main_Window::new_cb, mw, 0, MENU_BAR_STYLE},
			{"&Open..."_P, FL_COMMAND + 'o', (Fl_Callback *)Main_Window::open_cb, mw, 0, MENU_BAR_STYLE},
			{"&Close"_P, FL_COMMAND + 'w', (Fl_Callback *)Main_Window::close_cb, mw, FL_MENU_DIVIDER, MENU_BAR_STYLE},
			{"&Save..."_P, FL_COMMAND + 's', (Fl_Callback *)Main_Window::save_cb, mw, FL_MENU_DIVIDER, MENU_BAR_STYLE},
			{"E&xit"_P, FL_ALT + FL_F + 4, (Fl_Callback *)Main_Window::exit_cb, mw, 0, MENU_BAR_STYLE},
			{0},
		{"&Edit", 0, NULL, NULL, FL_SUBMENU, MENU_BAR_STYLE},
			{"&Decimate..."_P, FL_COMMAND + 'd', (Fl_Callback *)Main_Window::decimate_cb, mw, 0, MENU_BAR_STYLE},
			{"&Expand..."_P, FL_COMMAND + 'e', (Fl_Callback *)Main_Window::expand_cb, mw, 0, MENU_BAR_STYLE},
			{"&Interpolate..."_P, FL_COMMAND + 'i', (Fl_Callback *)Main_Window::interpolate_cb, mw, 0, MENU_BAR_STYLE},
			{"E&rode..."_P, FL_COMMAND + 'r', (Fl_Callback *)Main_Window::erode_cb, mw, 0, MENU_BAR_STYLE},
			{0},
		{"&View", 0, NULL, NULL, FL_SUBMENU, MENU_BAR_STYLE},
			{"&Toolbar"_P, FL_COMMAND + '\\', (Fl_Callback *)Main_Window::toolbar_cb, mw, FL_MENU_TOGGLE | FL_MENU_VALUE, MENU_BAR_STYLE},
			{"&Status Bar"_P, FL_COMMAND + '/', (Fl_Callback *)Main_Window::status_bar_cb, mw, FL_MENU_TOGGLE | FL_MENU_VALUE, MENU_BAR_STYLE},
			{"&Full Screen"_P, FL_F + 11, (Fl_Callback *)Main_Window::full_screen_cb, mw, FL_MENU_TOGGLE | FL_MENU_DIVIDER, MENU_BAR_STYLE},
			{"Zoom &In"_P, FL_COMMAND + '=', (Fl_Callback *)Main_Window::zoom_in_cb, mw, 0, MENU_BAR_STYLE},
			{"Zoom &Out"_P, FL_COMMAND + '-', (Fl_Callback *)Main_Window::zoom_out_cb, mw, 0, MENU_BAR_STYLE},
			{"Zoom &Reset"_P, FL_COMMAND + '0', (Fl_Callback *)Main_Window::zoom_reset_cb, mw, FL_MENU_DIVIDER, MENU_BAR_STYLE},
			{"&Normals"_P, FL_COMMAND + 'm', (Fl_Callback *)Main_Window::normals_cb, mw, 0, MENU_BAR_STYLE},
			{"Render &3D"_P, FL_COMMAND + '3', (Fl_Callback *)Main_Window::render_3d_cb, mw, FL_MENU_TOGGLE | FL_MENU_DIVIDER, MENU_BAR_STYLE},
			{"&Color Scheme..."_P, 0, NULL, NULL, FL_SUBMENU, MENU_BAR_STYLE},
				{"Gra&yscale"_P, FL_ALT + '0', (Fl_Callback *)Main_Window::color_scheme_cb, mw, FL_MENU_RADIO | FL_MENU_VALUE, MENU_BAR_STYLE},
				{"&Elevation (Red)"_P, FL_ALT + '1', (Fl_Callback *)Main_Window::color_scheme_cb, mw, FL_MENU_RADIO, MENU_BAR_STYLE},
				{"&Hardness (Green)"_P, FL_ALT + '2', (Fl_Callback *)Main_Window::color_scheme_cb, mw, FL_MENU_RADIO, MENU_BAR_STYLE},
				{"&Solubility (Blue)"_P, FL_ALT + '3', (Fl_Callback *)Main_Window::color_scheme_cb, mw, FL_MENU_RADIO, MENU_BAR_STYLE},
				{"&Combination (White)"_P, FL_ALT + '4', (Fl_Callback *)Main_Window::color_scheme_cb, mw, FL_MENU_RADIO, MENU_BAR_STYLE},
				{"&Artificial (Earth)"_P, FL_ALT + '5', (Fl_Callback *)Main_Window::color_scheme_cb, mw, FL_MENU_RADIO, MENU_BAR_STYLE},
				{0},
			{0},
		{"&Help", 0, NULL, NULL, FL_SUBMENU, MENU_BAR_STYLE},
			{"&About..."_P, FL_COMMAND + 'a', (Fl_Callback *)Main_Window::about_cb, mw, 0, MENU_BAR_STYLE},
			{0},
		{0}
	};
	copy(menu_items);
	// Initalize menu items
	_render_3d = const_cast<Fl_Menu_Item *>(find_item("&View/Render &3D"_P));
	_grayscale_mi = const_cast<Fl_Menu_Item *>(find_item("&View/&Color Scheme..."_P"/Gra&yscale"_P));
	_elevation_red_mi = const_cast<Fl_Menu_Item *>(find_item("&View/&Color Scheme..."_P"/&Elevation (Red)"_P));
	_hardness_green_mi = const_cast<Fl_Menu_Item *>(find_item("&View/&Color Scheme..."_P"/&Hardness (Green)"_P));
	_solubility_blue_mi = const_cast<Fl_Menu_Item *>(find_item("&View/&Color Scheme..."_P"/&Solubility (Blue)"_P));
	_combination_white_mi = const_cast<Fl_Menu_Item *>(find_item("&View/&Color Scheme..."_P"/&Combination (White)"_P));
	_artificial_earth_mi = const_cast<Fl_Menu_Item *>(find_item("&View/&Color Scheme..."_P"/&Artificial (Earth)"_P));
}

Color_Scheme Menu_Bar::menu_bar_color_scheme() const {
	if (_grayscale_mi->value()) { return GRAYSCALE; }
	if (_elevation_red_mi->value()) { return ELEVATION_RED; }
	if (_hardness_green_mi->value()) { return HARDNESS_GREEN; }
	if (_solubility_blue_mi->value()) { return SOLUBILITY_BLUE; }
	if (_combination_white_mi->value()) { return COMBINATION_WHITE; }
	if (_artificial_earth_mi->value()) { return ARTIFICIAL_EARTH; }
	return GRAYSCALE;
}

void Menu_Bar::menu_bar_color_scheme(Color_Scheme cs) {
	switch (cs) {
	default:
	case GRAYSCALE:
		_grayscale_mi->setonly();
		break;
	case ELEVATION_RED:
		_elevation_red_mi->setonly();
		break;
	case HARDNESS_GREEN:
		_hardness_green_mi->setonly();
		break;
	case SOLUBILITY_BLUE:
		_solubility_blue_mi->setonly();
		break;
	case COMBINATION_WHITE:
		_combination_white_mi->setonly();
		break;
	case ARTIFICIAL_EARTH:
		_artificial_earth_mi->setonly();
		break;
	}
}

#undef MENU_BAR_STYLE
#undef _P
