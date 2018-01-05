#pragma once

#include <cstdlib>

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Native_File_Chooser.H>
#pragma warning(pop)

#include "menu-bar.h"
#include "toolbar.h"
#include "status-bar.h"
#include "workspace.h"
#include "file-choosers.h"
#include "modal-dialogs.h"

class Main_Window : public Fl_Double_Window {
private:
	Menu_Bar *_menu_bar;
	Toolbar *_toolbar;
	Status_Bar *_status_bar;
	Workspace *_workspace;
	Alert_Dialog *_error_dialog, *_success_dialog, *_info_dialog;
	Progress_Dialog *_progress_dialog;
	Alert_Dialog *_about_dialog;
	New_DTED_Dialog *_new_dted_dialog;
	Decimation_Dialog *_decimation_dialog;
	Expansion_Dialog *_expansion_dialog;
	Interpolation_Dialog *_interpolation_dialog;
	Erosion_Dialog *_erosion_dialog;
	Open_DTED_Chooser *_open_dted_chooser;
	Save_DTED_Chooser *_save_dted_chooser;
public:
	Main_Window(int x, int y, int w, int h, const char *l = NULL);
	void show(int argc, char **argv);
private:
	void refresh_file(const char *filename);
	void refresh_status(void);
	inline void render_3d(bool r) { _workspace->render_3d(r); redraw(); }
	inline void color_scheme(Color_Scheme cs) { _workspace->color_scheme(cs); redraw(); }
public:
	static void new_cb(Fl_Widget *w, Main_Window *mw);
	static void open_cb(Fl_Widget *w, Main_Window *mw);
	static void close_cb(Fl_Widget *w, Main_Window *mw);
	static void save_cb(Fl_Widget *w, Main_Window *mw);
	static void exit_cb(Fl_Widget *w, void *v);
	static void decimate_cb(Fl_Widget *w, Main_Window *mw);
	static void expand_cb(Fl_Widget *w, Main_Window *mw);
	static void interpolate_cb(Fl_Widget *w, Main_Window *mw);
	static void erode_cb(Fl_Widget *w, Main_Window *mw);
	static void toolbar_cb(Fl_Menu_ *m, Main_Window *mw);
	static void status_bar_cb(Fl_Menu_ *m, Main_Window *mw);
	static void full_screen_cb(Fl_Menu_ *m, Main_Window *mw);
	static void zoom_in_cb(Fl_Menu_ *m, Main_Window *mw);
	static void zoom_out_cb(Fl_Menu_ *m, Main_Window *mw);
	static void zoom_reset_cb(Fl_Menu_ *m, Main_Window *mw);
	static void color_scheme_cb(Fl_Menu_ *m, Main_Window *mw);
	static void color_scheme_tb_cb(Fl_Widget *w, Main_Window *mw);
	static void render_3d_cb(Fl_Menu_ *m, Main_Window *mw);
	static void render_3d_tb_cb(Fl_Widget *w, Main_Window *mw);
	static void normals_cb(Fl_Menu_ *m, Main_Window *mw);
	static void scale_cb(Fl_Widget *w, Main_Window *mw);
	static void about_cb(Fl_Widget *w, Main_Window *mw);
};
