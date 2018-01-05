#pragma once

#include <cstdlib>

#include "widgets.h"

class Status_Bar : public Fl_Toolbar {
private:
	Fl_Status_Bar_Field *_dimensions, *_num_points;
public:
	Status_Bar(int ww, int wh);
	void status(size_t ww, size_t hh, size_t n);
	void reset(void);
};
