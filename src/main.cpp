#include <cstdlib>
#include <iostream>
#include <ctime>

#pragma warning(push, 0)
#include <FL/Fl.H>
#pragma warning(pop)

#include "os-font.h"
#include "algebra.h"
#include "main-window.h"

int main(int argc, char **argv) {
	std::ios::sync_with_stdio(false);
	srand((unsigned int)time(NULL));
	use_os_font();
	Main_Window *main_window = new Main_Window(0, 0, 1600, 1200); // allow space for components to lay out
	main_window->end();
	Fl::visual(FL_DOUBLE | FL_INDEX);
	int ww = MIN(1152, Fl::w() - 150), hh = MIN(800, Fl::h() - 150);
	int xx = (Fl::w() - ww) / 2, yy = (Fl::h() - hh) / 2;
	main_window->resize(xx, yy, ww, hh);
	main_window->show(argc, argv);
	return Fl::run();
}
