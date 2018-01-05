#include <cstdlib>
#include <sstream>

#include "widgets.h"
#include "status-bar.h"

Status_Bar::Status_Bar(int ww, int wh) : Fl_Toolbar(0, wh-23, ww, 23, NULL) {
	// Populate status bar
	_dimensions = new Fl_Status_Bar_Field(0, 0, 100, 24, "");
	_num_points = new Fl_Status_Bar_Field(0, 0, 300, 24, "");
	// Initialize status bar
	spacing(0);
	clip_children(1);
	end();
}

void Status_Bar::status(size_t ww, size_t hh, size_t n) {
	std::ostringstream ss;
	ss.imbue(std::locale(""));
	ss.setf(std::ios::fixed, std::ios::floatfield);
	ss.precision(0);
	ss << ww << " x " << hh;
	_dimensions->copy_label(ss.str().c_str());
	ss.str("");
	size_t np = ww * hh;
	ss << n << " points / " << np << " possible";
	_num_points->copy_label(ss.str().c_str());
}

void Status_Bar::reset() {
	_dimensions->reset_label();
	_num_points->reset_label();
}
