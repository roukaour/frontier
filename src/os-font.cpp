#include <cstdlib>
#include <cstring>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#pragma warning(pop)

#include "os-font.h"

void use_os_font() {
	Fl::set_font(OS_FONT, FL_HELVETICA);
	int num_fonts = Fl::set_fonts(NULL);
	for (Fl_Font f = 0; f < num_fonts; f++) {
		const char *name = Fl::get_font_name(f);
		if (!strcmp(name, OS_FONT_NAME_ALT)) {
			Fl::set_font(OS_FONT, name);
		}
		if (!strcmp(name, OS_FONT_NAME)) {
			Fl::set_font(OS_FONT, name);
			break;
		}
	}
	fl_font(OS_FONT, OS_FONT_SIZE);
}
