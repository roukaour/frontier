#include <iostream>
#include <sstream>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#pragma warning(pop)

#include "metadata.h"
#include "menu-bar.h"
#include "toolbar.h"
#include "status-bar.h"
#include "workspace.h"
#include "file-choosers.h"
#include "main-window.h"

#ifdef _WIN32
#include "resource.h"
#endif

Main_Window::Main_Window(int x, int y, int w, int h, const char *) : Fl_Double_Window(x, y, w, h, TERRAIN_PROGRAM_NAME) {
	// Populate window
	_menu_bar = new Menu_Bar(w, h);
	_toolbar = new Toolbar(w, h);
	_status_bar = new Status_Bar(w, h);
	_workspace = new Workspace(0, _menu_bar->h() + _toolbar->h(), w, h - _menu_bar->h() - _toolbar->h() - _status_bar->h());
	_error_dialog = new Alert_Dialog("Error", Alert_Dialog::ERROR_ICON);
	_success_dialog = new Alert_Dialog("Success", Alert_Dialog::SUCCESS_ICON);
	_info_dialog = new Alert_Dialog("Information", Alert_Dialog::INFO_ICON);
	_progress_dialog = new Progress_Dialog("Progress...");
	_about_dialog = new Alert_Dialog("About " TERRAIN_PROGRAM_NAME, Alert_Dialog::PROGRAM_ICON);
	_new_dted_dialog = new New_DTED_Dialog("New DTED...");
	_decimation_dialog = new Decimation_Dialog("Decimate...");
	_expansion_dialog = new Expansion_Dialog("Expand...");
	_interpolation_dialog = new Interpolation_Dialog("Interpolate...");
	_erosion_dialog = new Erosion_Dialog("Erode...");
	// Initialize dialogs
	_about_dialog->min_size(320, 104);
	_about_dialog->subject(TERRAIN_PROGRAM_NAME " " TERRAIN_VERSION_STRING);
	_about_dialog->message("Built at " __TIME__ ", " __DATE__ ".\n\n"
		"Copyright (C) Remy Oukaour 2013\n"
		"for CSE 528 with Prof. Hong Qin,\n"
		"Stony Brook University.\n\n"
		"Icons are from the Fugue icon set\nby Yusuke Kamiyamane.");
	// Initialize file choosers
	_open_dted_chooser = new Open_DTED_Chooser();
	_save_dted_chooser = new Save_DTED_Chooser();
	// Initialize window
	resizable(_workspace);
	callback(exit_cb);
#ifdef _WIN32
	icon((char *)LoadIcon(fl_display, MAKEINTRESOURCE(IDI_ICON1)));
#endif
}

void Main_Window::show(int argc, char **argv) {
	Fl_Double_Window::show(argc, argv);
#ifdef _WIN32
	// Fix for 16x16 icon from <http://www.fltk.org/str.php?L925>
	HANDLE big_icon = LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON,
		GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CXICON), 0);
	SendMessage(fl_xid(this), WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(big_icon));
	HANDLE small_icon = LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CXSMICON), 0);
	SendMessage(fl_xid(this), WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(small_icon));
#endif
	_workspace->show(); // fix for invalid context
}

void Main_Window::refresh_file(const char *filename) {
	if (filename) {
		// Refresh title bar
		std::ostringstream ss;
		const Heightmap &hm = _workspace->heightmap();
		ss << filename << " (" << hm.width() << "x" << hm.height() << ") - " << TERRAIN_PROGRAM_NAME;
		copy_label(ss.str().c_str());
		// Refresh status bar
		refresh_status();
	}
	else {
		// Refresh title bar
		label(TERRAIN_PROGRAM_NAME);
		// Refresh workspace
		_workspace->clear();
		// Refresh status bar
		_status_bar->reset();
	}
	_workspace->redraw();
	redraw();
}

void Main_Window::refresh_status() {
	const Heightmap &hm = _workspace->heightmap();
	size_t ww = hm.width(), hh = hm.height(), n = hm.known_elevations();
	_status_bar->status(ww, hh, n);
	redraw();
}

void Main_Window::new_cb(Fl_Widget *, Main_Window *mw) {
	mw->_new_dted_dialog->show(mw);
	if (mw->_new_dted_dialog->canceled()) { return; }
	size_t width = mw->_new_dted_dialog->dted_width();
	size_t height = mw->_new_dted_dialog->dted_height();
	bool success = mw->_workspace->create(width, height);
	if (!success) {
		std::ostringstream ss;
		ss << "Could not create new DTED!";
		mw->_error_dialog->message(strdup(ss.str().c_str()), true);
		mw->_error_dialog->show(mw);
		return;
	}
	mw->refresh_file("Untitled");
}

void Main_Window::open_cb(Fl_Widget *, Main_Window *mw) {
	int status = mw->_open_dted_chooser->show();
	if (status == 1) { return; }
	const char *filename = mw->_open_dted_chooser->filename();
	const char *basename = fl_filename_name(filename);
	if (status == -1) {
		std::ostringstream ss;
		ss << "Could not open " << basename << "!";
		mw->_error_dialog->message(strdup(ss.str().c_str()), true);
		mw->_error_dialog->show(mw);
		return;
	}
	bool success = mw->_workspace->open(filename);
	if (!success) {
		std::ostringstream ss;
		ss << "Could not load " << basename << " as elevation data!";
		mw->_error_dialog->message(strdup(ss.str().c_str()), true);
		mw->_error_dialog->show(mw);
		return;
	}
	mw->refresh_file(basename);
}

void Main_Window::close_cb(Fl_Widget *, Main_Window *mw) {
	mw->_workspace->close();
	mw->refresh_file(NULL);
}

void Main_Window::save_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_workspace->opened()) { return; }
	int status = mw->_save_dted_chooser->show();
	if (status == 1) { return; }
	const char *filename = mw->_save_dted_chooser->filename();
	const char *basename = fl_filename_name(filename);
	mw->_progress_dialog->title("Saving...");
	mw->_progress_dialog->show(mw);
	bool success = mw->_workspace->save(filename, mw->_progress_dialog);
	mw->_progress_dialog->hide();
	if (mw->_progress_dialog->canceled()) {
		std::ostringstream ss;
		ss << "Canceled saving " << basename << "!";
		mw->_info_dialog->message(strdup(ss.str().c_str()), true);
		mw->_info_dialog->show(mw);
	}
	else if (!success) {
		std::ostringstream ss;
		ss << "Could not save " << basename << "!";
		mw->_error_dialog->message(strdup(ss.str().c_str()), true);
		mw->_error_dialog->show(mw);
	}
	else {
		std::ostringstream ss;
		ss << "Saved " << basename << "!";
		mw->_success_dialog->message(strdup(ss.str().c_str()), true);
		mw->_success_dialog->show(mw);
	}
}

void Main_Window::exit_cb(Fl_Widget *, void *) {
	// Override default behavior of Esc to close main window
	if (Fl::event() == FL_SHORTCUT && Fl::event_key() == FL_Escape) { return; }
	exit(EXIT_SUCCESS);
}

void Main_Window::decimate_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_workspace->opened()) { return; }
	mw->_decimation_dialog->show(mw);
	if (mw->_decimation_dialog->canceled()) { return; }
	bool random = mw->_decimation_dialog->keep_random();
	double threshold = mw->_decimation_dialog->decimation_threshold();
	mw->_progress_dialog->title("Decimating...");
	mw->_progress_dialog->show(mw);
	mw->_workspace->decimate(random, threshold, mw->_progress_dialog);
	mw->_progress_dialog->hide();
	if (mw->_progress_dialog->canceled()) {
		std::ostringstream ss;
		ss << "Canceled decimating!";
		mw->_info_dialog->message(strdup(ss.str().c_str()), true);
		mw->_info_dialog->show(mw);
	}
	else {
		std::ostringstream ss;
		ss << "Decimated!";
		mw->_success_dialog->message(strdup(ss.str().c_str()), true);
		mw->_success_dialog->show(mw);
	}
	mw->refresh_status();
}

void Main_Window::expand_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_workspace->opened()) { return; }
	mw->_expansion_dialog->show(mw);
	if (mw->_expansion_dialog->canceled()) { return; }
	size_t power = mw->_expansion_dialog->expansion_power();
	mw->_progress_dialog->title("Expanding...");
	mw->_progress_dialog->show(mw);
	bool success = mw->_workspace->expand(power, mw->_progress_dialog);
	mw->_progress_dialog->hide();
	if (mw->_progress_dialog->canceled()) {
		std::ostringstream ss;
		ss << "Canceled expanding!";
		mw->_info_dialog->message(strdup(ss.str().c_str()), true);
		mw->_info_dialog->show(mw);
	}
	else if (!success) {
		std::ostringstream ss;
		ss << "Could not expand!";
		mw->_error_dialog->message(strdup(ss.str().c_str()), true);
		mw->_error_dialog->show(mw);
	}
	else {
		std::ostringstream ss;
		ss << "Expanded!";
		mw->_success_dialog->message(strdup(ss.str().c_str()), true);
		mw->_success_dialog->show(mw);
	}
	mw->refresh_status();
}

void Main_Window::interpolate_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_workspace->opened()) { return; }
	mw->_interpolation_dialog->show(mw);
	if (mw->_interpolation_dialog->canceled()) { return; }
	bool mdbu = mw->_interpolation_dialog->mdbu_process();
	float I = mw->_interpolation_dialog->param_I();
	bool md = mw->_interpolation_dialog->md_process();
	float H = mw->_interpolation_dialog->param_H();
	float rt = mw->_interpolation_dialog->param_rt();
	float rs = mw->_interpolation_dialog->param_rs();
	mw->_progress_dialog->title("Interpolating...");
	mw->_progress_dialog->show(mw);
	mw->_workspace->interpolate(mdbu, I, md, H, rt, rs, mw->_progress_dialog);
	mw->_progress_dialog->hide();
	if (mw->_progress_dialog->canceled()) {
		std::ostringstream ss;
		ss << "Canceled interpolating!";
		mw->_info_dialog->message(strdup(ss.str().c_str()), true);
		mw->_info_dialog->show(mw);
	}
	else {
		std::ostringstream ss;
		ss << "Interpolated!";
		mw->_success_dialog->message(strdup(ss.str().c_str()), true);
		mw->_success_dialog->show(mw);
	}
	mw->refresh_status();
}

void Main_Window::erode_cb(Fl_Widget *, Main_Window *mw) {
	if (!mw->_workspace->opened()) { return; }
	mw->_erosion_dialog->show(mw);
	if (mw->_erosion_dialog->canceled()) { return; }
	size_t nts = mw->_erosion_dialog->param_nts();
	bool thermal = mw->_erosion_dialog->thermal_erosion();
	float Kt = mw->_erosion_dialog->param_Kt();
	float Ka = mw->_erosion_dialog->param_Ka();
	float Ki = mw->_erosion_dialog->param_Ki();
	bool hydraulic = mw->_erosion_dialog->hydraulic_erosion();
	float Kc = mw->_erosion_dialog->param_Kc();
	float Kd = mw->_erosion_dialog->param_Kd();
	float Ks = mw->_erosion_dialog->param_Ks();
	float Ke = mw->_erosion_dialog->param_Ke();
	float W0 = mw->_erosion_dialog->param_W0();
	float Wmin = mw->_erosion_dialog->param_Wmin();
	mw->_progress_dialog->title("Eroding...");
	mw->_progress_dialog->show(mw);
	mw->_workspace->erode(nts, thermal, Kt, Ka, Ki, hydraulic, Kc, Kd, Ks, Ke, W0, Wmin, mw->_progress_dialog);
	mw->_progress_dialog->hide();
	if (mw->_progress_dialog->canceled()) {
		std::ostringstream ss;
		ss << "Canceled eroding!";
		mw->_info_dialog->message(strdup(ss.str().c_str()), true);
		mw->_info_dialog->show(mw);
	}
	else {
		std::ostringstream ss;
		ss << "Eroded!";
		mw->_success_dialog->message(strdup(ss.str().c_str()), true);
		mw->_success_dialog->show(mw);
	}
	mw->refresh_status();
}

void Main_Window::toolbar_cb(Fl_Menu_ *m, Main_Window *mw) {
	int dy = mw->_toolbar->h();
	if (m->mvalue()->value()) {
		mw->add(mw->_toolbar);
		mw->_toolbar->size(mw->w(), dy);
		mw->_workspace->resize(mw->_workspace->x(), mw->_workspace->y() + dy,
			mw->_workspace->w(), mw->_workspace->h() - dy);
	}
	else {
		mw->remove(mw->_toolbar);
		mw->_workspace->resize(mw->_workspace->x(), mw->_workspace->y() - dy,
			mw->_workspace->w(), mw->_workspace->h() + dy);
	}
	mw->redraw();
}

void Main_Window::status_bar_cb(Fl_Menu_ *m, Main_Window *mw) {
	int dy = mw->_status_bar->h();
	if (m->mvalue()->value()) {
		mw->add(mw->_status_bar);
		mw->_status_bar->resize(mw->_status_bar->x(), mw->h() - dy, mw->w(), dy);
		mw->_workspace->size(mw->_workspace->w(), mw->_workspace->h() - dy);
	}
	else {
		mw->remove(mw->_status_bar);
		mw->_workspace->size(mw->_workspace->w(), mw->_workspace->h() + dy);
	}
	mw->redraw();
}

void Main_Window::full_screen_cb(Fl_Menu_ *m, Main_Window *mw) {
	if (m->mvalue()->value()) { mw->fullscreen(); }
	else { mw->fullscreen_off(); }
}

void Main_Window::zoom_in_cb(Fl_Menu_ *, Main_Window *mw) {
	mw->_workspace->zoom_in(mw->_workspace->w() / 2, mw->_workspace->h() / 2);
}

void Main_Window::zoom_out_cb(Fl_Menu_ *, Main_Window *mw) {
	mw->_workspace->zoom_out(mw->_workspace->w() / 2, mw->_workspace->h() / 2);
}

void Main_Window::zoom_reset_cb(Fl_Menu_ *, Main_Window *mw) {
	mw->_workspace->zoom_reset(mw->_workspace->w() / 2, mw->_workspace->h() / 2);
}

void Main_Window::color_scheme_cb(Fl_Menu_ *, Main_Window *mw) {
	Color_Scheme cs = mw->_menu_bar->menu_bar_color_scheme();
	mw->_toolbar->toolbar_color_scheme(cs);
	mw->color_scheme(cs);
}

void Main_Window::color_scheme_tb_cb(Fl_Widget *, Main_Window *mw) {
	Color_Scheme cs = mw->_toolbar->toolbar_color_scheme();
	mw->_menu_bar->menu_bar_color_scheme(cs);
	mw->color_scheme(cs);
}

void Main_Window::render_3d_cb(Fl_Menu_ *, Main_Window *mw) {
	bool r = mw->_menu_bar->menu_bar_render_3d();
	mw->_toolbar->toolbar_render_3d(r);
	mw->render_3d(r);
}

void Main_Window::render_3d_tb_cb(Fl_Widget *, Main_Window *mw) {
	bool r = mw->_toolbar->toolbar_render_3d();
	mw->_menu_bar->menu_bar_render_3d(r);
	mw->render_3d(r);
}

void Main_Window::normals_cb(Fl_Menu_ *, Main_Window *mw) {
	mw->_progress_dialog->title("Calculating normals...");
	mw->_progress_dialog->show(mw);
	bool success = mw->_workspace->calculate_normals(mw->_progress_dialog);
	mw->_progress_dialog->hide();
	if (mw->_progress_dialog->canceled()) {
		std::ostringstream ss;
		ss << "Canceled calculating normals!";
		mw->_info_dialog->message(strdup(ss.str().c_str()), true);
		mw->_info_dialog->show(mw);
	}
	else if (!success) {
		std::ostringstream ss;
		ss << "Could not calculate normals!";
		mw->_error_dialog->message(strdup(ss.str().c_str()), true);
		mw->_error_dialog->show(mw);
	}
	else {
		std::ostringstream ss;
		ss << "Calculated normals!";
		mw->_success_dialog->message(strdup(ss.str().c_str()), true);
		mw->_success_dialog->show(mw);
	}
}

void Main_Window::scale_cb(Fl_Widget *, Main_Window *mw) {
	mw->_workspace->scale(mw->_toolbar->toolbar_scale());
}

void Main_Window::about_cb(Fl_Widget *, Main_Window *mw) {
	mw->_about_dialog->show(mw);
}
