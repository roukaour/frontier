#include <string>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Check_Button.H>
#pragma warning(pop)

#include "os-font.h"
#include "icons.h"
#include "widgets.h"
#include "modal-dialogs.h"

Modal_Dialog::Modal_Dialog(const char *t, Dialog_Type dt) : _dialog_type(dt), _title(t), _min_w(280), _min_h(104),
	_canceled(false), _dialog(NULL), _ok_button(NULL), _cancel_button(NULL), _spacer(NULL) {}

Modal_Dialog::~Modal_Dialog() {
	delete _title;
	delete _dialog;
	delete _ok_button;
	delete _cancel_button;
	delete _spacer;
}

void Modal_Dialog::initialize() {
	_canceled = false;
	if (_dialog) { return; }
	Fl_Group *prev_current = Fl_Group::current();
	Fl_Group::current(NULL);
	// Populate dialog
	_dialog = new Fl_Double_Window(0, 0, _min_w, _min_h, _title);
	on_initialize();
	_ok_button = _dialog_type | OK_DIALOG ? new Fl_Button(0, 0, 0, 0, "OK") : NULL;
	_cancel_button = _dialog_type | CANCEL_DIALOG ? new Fl_Button(0, 0, 0, 0, "Cancel") : NULL;
	_spacer = new Fl_Box(0, 0, 0, 0);
	_dialog->end();
	// Initialize dialog
	_dialog->resizable(_spacer);
	_dialog->size_range(_min_w, _min_h, _min_w, _min_h);
	_dialog->callback((Fl_Callback *)close_cb, this);
	_dialog->set_modal();
	// Initialize OK button
	if (_ok_button) {
		_ok_button->labelfont(OS_FONT);
		_ok_button->labelsize(OS_FONT_SIZE);
		_ok_button->shortcut(FL_Enter);
		_ok_button->callback((Fl_Callback *)close_cb, this);
	}
	// Initialize Cancel button
	if (_cancel_button) {
		_cancel_button->labelfont(OS_FONT);
		_cancel_button->labelsize(OS_FONT_SIZE);
		_cancel_button->shortcut(FL_Escape);
		_cancel_button->callback((Fl_Callback *)cancel_cb, this);
	}
	Fl_Group::current(prev_current);
}

void Modal_Dialog::show(const Fl_Widget *p, bool wait) {
	on_show(p);
	initialize();
	refresh();
	if (_ok_button) { _ok_button->take_focus(); }
	else if (_cancel_button) { _cancel_button->take_focus(); }
	int x = p->x() + (p->w() - _dialog->w()) / 2;
	int y = p->y() + (p->h() - _dialog->h()) / 2;
	_dialog->position(x, y);
	_dialog->show();
	if (wait) { while (_dialog->shown()) { Fl::wait(); } }
}

void Modal_Dialog::close_cb(Fl_Widget *, Modal_Dialog *md) { md->hide(); }

void Modal_Dialog::cancel_cb(Fl_Widget *, Modal_Dialog *md) { md->canceled(true); md->hide(); }

Alert_Dialog::Alert_Dialog(const char *t, Icon_Type it) : Modal_Dialog(t, OK_DIALOG), _icon_type(it), _subject(NULL),
	_message(NULL), _wrap_body(true), _icon(NULL), _heading(NULL), _body(NULL) {}

Alert_Dialog::~Alert_Dialog() {
	delete _icon;
	delete _heading;
	delete _body;
}

void Alert_Dialog::on_initialize() {
	_icon = new Fl_Box(0, 0, 0, 0);
	_heading = new Fl_Box(0, 0, 0, 0, _subject);
	_body = new Fl_Box(0, 0, 0, 0);
	// Initialize icon
	_icon->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	// Initialize heading
	_heading->labelfont(OS_FONT);
	_heading->labelsize(OS_FONT_SIZE + 4);
	_heading->align(FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
	// Initialize body
	_body->labelfont(OS_FONT);
	_body->labelsize(OS_FONT_SIZE);
	_body->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
}

static int count(const char *s, char c) {
	int n = 0;
	for (const char *m = s; *m; m++) {
		if (*m == c) { n++; }
	}
	return n;
}

void Alert_Dialog::refresh() {
	// Refresh widget labels
	_heading->label(_subject);
	_dialog->label(_title);
	_body->label(_message);
	// Refresh icon
	switch (_icon_type) {
	case NO_ICON:
		_icon->label(NULL);
		_icon->labelcolor(FL_FOREGROUND_COLOR);
		_icon->color(FL_BACKGROUND_COLOR);
		_icon->image(NULL);
		_icon->box(FL_NO_BOX);
		break;
	case INFO_ICON:
		_icon->label("i");
		_icon->labelfont(FL_HELVETICA_BOLD);
		_icon->labelsize(40);
		_icon->labelcolor(FL_BLUE);
		_icon->color(FL_WHITE);
		_icon->image(NULL);
		_icon->box(FL_THIN_UP_BOX);
		break;
	case QUESTION_ICON:
		_icon->label("?");
		_icon->labelfont(FL_HELVETICA_BOLD);
		_icon->labelsize(40);
		_icon->labelcolor(FL_WHITE);
		_icon->color(FL_BLUE);
		_icon->image(NULL);
		_icon->box(FL_THIN_UP_BOX);
		break;
	case WARNING_ICON:
		_icon->label("!");
		_icon->labelfont(FL_COURIER_BOLD);
		_icon->labelsize(40);
		_icon->labelcolor(FL_BLACK);
		_icon->color(FL_YELLOW);
		_icon->image(NULL);
		_icon->box(FL_ROUND_UP_BOX);
		break;
	case ERROR_ICON:
		_icon->label("X");
		_icon->labelfont(FL_COURIER_BOLD);
		_icon->labelsize(40);
		_icon->labelcolor(FL_WHITE);
		_icon->color(FL_RED);
		_icon->image(NULL);
		_icon->box(FL_ROUND_UP_BOX);
		break;
	case SUCCESS_ICON:
		_icon->label("\xE2\x9C\x93"); // UTF-8 encoding for U+2713 "CHECK MARK"
		_icon->labelfont(FL_HELVETICA_BOLD);
		_icon->labelsize(40);
		_icon->labelcolor(FL_WHITE);
		_icon->color(FL_DARK_GREEN);
		_icon->image(NULL);
		_icon->box(FL_ROUND_UP_BOX);
		break;
	case PROGRAM_ICON:
		_icon->label(NULL);
		_icon->labelfont(FL_HELVETICA_BOLD);
		_icon->labelsize(40);
		_icon->labelcolor(FL_BLACK);
		_icon->color(FL_BACKGROUND_COLOR);
		_icon->image(FRONTIER_ICON);
		_icon->box(FL_NO_BOX);
	}
	// Refresh body
	_body->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE | (_wrap_body ? FL_ALIGN_WRAP : 0));
	// Refresh widget positions and sizes
	if (_icon_type == NO_ICON) {
		_icon->resize(0, 0, 0, 0);
		_heading->resize(10, 10, _min_w-20, 24);
		_body->resize(10, _subject?44:10, _min_w-20, _min_h-88);
		_ok_button->resize((_min_w-80)/2, _min_h-34, 80, 24);
		_spacer->resize(10, _min_h-44, 1, 1);
	}
	else {
		_icon->resize(10, 10, 50, 50);
		_heading->resize(70, 10, _min_w-80, 24);
		_body->resize(70, _subject?44:10, _min_w-80, _min_h-88);
		_ok_button->resize((_min_w-140)/2+60, _min_h-34, 80, 24);
		_spacer->resize(_min_w, 60, 1, 1);
	}
	int h = (count(_body->label(), '\n') + 1) * (_body->labelsize() + 4) + 88;
	_body->size(_body->w(), h);
	if (h < _min_h) { h = _min_h; }
	_dialog->size_range(_min_w, h, _min_w, h);
	_dialog->size(_min_w, h);
	_dialog->redraw();
}

Progress_Dialog::Progress_Dialog(const char *t) : Modal_Dialog(t, CANCEL_DIALOG), _body(NULL), _progress(NULL) {}

Progress_Dialog::~Progress_Dialog() {
	delete _body;
	delete _progress;
}

void Progress_Dialog::on_initialize() {
	_body = new Fl_Box(0, 0, 0, 0);
	_progress = new Fl_Progress(0, 0, 0, 0);
	// Initialize body
	_body->labelfont(OS_FONT);
	_body->labelsize(OS_FONT_SIZE);
	_body->align(FL_ALIGN_TOP_LEFT | FL_ALIGN_INSIDE);
	// Initialize progress bar
	_progress->labelfont(OS_FONT);
	_progress->labelsize(OS_FONT_SIZE);
	_progress->box(FL_DOWN_BOX);
	_progress->color(FL_BACKGROUND_COLOR);
	_progress->selection_color(FL_GREEN);
	_progress->minimum(0.0f);
	_progress->maximum(1.0f);
	_progress->value(0.0f);
}

void Progress_Dialog::refresh() {
	// Refresh widget labels
	_dialog->label(_title);
	// Refresh widget positions and sizes
	_body->resize(10, 10, _min_w-20, 16);
	_progress->resize(10, 36, _min_w-20, 16);
	_cancel_button->resize(_min_w-90, _min_h-34, 80, 24);
	_spacer->resize(10, _min_h-44, 1, 1);
	int h = _body->labelsize() + 4 + 88;
	_body->size(_body->w(), h);
	if (h < _min_h) { h = _min_h; }
	_dialog->size_range(_min_w, h, _min_w, h);
	_dialog->size(_min_w, h);
	_dialog->redraw();
}

New_DTED_Dialog::New_DTED_Dialog(const char *t) : Modal_Dialog(t, OK_CANCEL_DIALOG), _width_spinner(NULL),
	_height_spinner(NULL), _width_spinner_units(NULL), _height_spinner_units(NULL) {}

New_DTED_Dialog::~New_DTED_Dialog() {
	delete _width_spinner;
	delete _height_spinner;
	delete _width_spinner_units;
	delete _height_spinner_units;
}

void New_DTED_Dialog::on_initialize() {
	_width_spinner = new Fl_Spinner(0, 0, 0, 0, "Width:");
	_height_spinner = new Fl_Spinner(0, 0, 0, 0, "Height:");
	_width_spinner_units = new Fl_Text(0, 0, 0, 0, "px");
	_height_spinner_units = new Fl_Text(0, 0, 0, 0, "px");
	// Initialize parameter controls
	_width_spinner->labelfont(OS_FONT);
	_width_spinner->labelsize(OS_FONT_SIZE);
	_width_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_width_spinner->textfont(OS_FONT);
	_width_spinner->textsize(OS_FONT_SIZE);
	_width_spinner->type(FL_INT_INPUT);
	_width_spinner->range(2.0, 1024.0);
	_width_spinner->step(1.0);
	_width_spinner->value(513.0);
	_height_spinner->labelfont(OS_FONT);
	_height_spinner->labelsize(OS_FONT_SIZE);
	_height_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_height_spinner->textfont(OS_FONT);
	_height_spinner->textsize(OS_FONT_SIZE);
	_height_spinner->type(FL_INT_INPUT);
	_height_spinner->range(2.0, 1024.0);
	_height_spinner->step(1.0);
	_height_spinner->value(513.0);
}

void New_DTED_Dialog::refresh() {
	// Refresh widget labels
	_dialog->label(_title);
	// Refresh widget positions and sizes
	_width_spinner->resize(56, 10, 48, 22);
	_width_spinner_units->resize(101, 10, 24, 22);
	_height_spinner->resize(56, 36, 48, 22);
	_height_spinner_units->resize(101, 36, 24, 22);
	_min_h = 104;
	_ok_button->resize(_min_w-180, _min_h-34, 80, 24);
	_cancel_button->resize(_min_w-90, _min_h-34, 80, 24);
	_spacer->resize(10, _min_h-44, 1, 1);
	_dialog->size_range(_min_w, _min_h, _min_w, _min_h);
	_dialog->size(_min_w, _min_h);
	_dialog->redraw();
}

Decimation_Dialog::Decimation_Dialog(const char *t) : Modal_Dialog(t, OK_CANCEL_DIALOG), _keep_label(NULL),
	_keep_random(NULL), _keep_edges(NULL), _percent_spinner(NULL), _percent_spinner_units(NULL) {}

Decimation_Dialog::~Decimation_Dialog() {
	delete _keep_label;
	delete _keep_random;
	delete _keep_edges;
	delete _percent_spinner;
	delete _percent_spinner_units;
}

void Decimation_Dialog::on_initialize() {
	_keep_label = new Fl_Text(0, 0, 0, 0, "Keep:");
	_keep_random = new Fl_Radio_Round_Button(0, 0, 0, 0, "Random");
	_keep_edges = new Fl_Radio_Round_Button(0, 0, 0, 0, "Edges");
	_percent_spinner = new Fl_Spinner(0, 0, 0, 0, "Percent:");
	_percent_spinner_units = new Fl_Text(0, 0, 0, 0, "%");
	// Initialize parameter controls
	_keep_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	_keep_random->labelfont(OS_FONT);
	_keep_random->labelsize(OS_FONT_SIZE);
	_keep_random->setonly();
	_keep_edges->labelfont(OS_FONT);
	_keep_edges->labelsize(OS_FONT_SIZE);
	_percent_spinner->labelfont(OS_FONT);
	_percent_spinner->labelsize(OS_FONT_SIZE);
	_percent_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_percent_spinner->textfont(OS_FONT);
	_percent_spinner->textsize(OS_FONT_SIZE);
	_percent_spinner->type(FL_INT_INPUT);
	_percent_spinner->range(1.0, 99.0);
	_percent_spinner->step(1.0);
	_percent_spinner->value(50.0);
}

void Decimation_Dialog::refresh() {
	// Refresh widget labels
	_dialog->label(_title);
	// Refresh widget positions and sizes
	_keep_label->resize(10, 10, 32, 22);
	_keep_random->resize(47, 10, 65, 22);
	_keep_edges->resize(117, 10, 60, 22);
	_percent_spinner->resize(56, 36, 48, 22);
	_percent_spinner_units->resize(101, 36, 24, 22);
	_min_h = 104;
	_ok_button->resize(_min_w-180, _min_h-34, 80, 24);
	_cancel_button->resize(_min_w-90, _min_h-34, 80, 24);
	_spacer->resize(9, _min_h-44, 1, 1);
	_dialog->size_range(_min_w, _min_h, _min_w, _min_h);
	_dialog->size(_min_w, _min_h);
	_dialog->redraw();
}

Expansion_Dialog::Expansion_Dialog(const char *t) : Modal_Dialog(t, OK_CANCEL_DIALOG), _power_spinner(NULL) {}

Expansion_Dialog::~Expansion_Dialog() {
	delete _power_spinner;
}

void Expansion_Dialog::on_initialize() {
	_power_spinner = new Fl_Spinner(0, 0, 0, 0, "Factor: 2^");
	// Initialize parameter controls
	_power_spinner->labelfont(OS_FONT);
	_power_spinner->labelsize(OS_FONT_SIZE);
	_power_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_power_spinner->textfont(OS_FONT);
	_power_spinner->textsize(OS_FONT_SIZE);
	_power_spinner->type(FL_INT_INPUT);
	_power_spinner->range(1.0, 5.0);
	_power_spinner->step(1.0);
	_power_spinner->value(2.0);
}

void Expansion_Dialog::refresh() {
	// Refresh widget labels
	_dialog->label(_title);
	// Refresh widget positions and sizes
	_power_spinner->resize(66, 10, 48, 22);
	_min_h = 104;
	_ok_button->resize(_min_w-180, _min_h-34, 80, 24);
	_cancel_button->resize(_min_w-90, _min_h-34, 80, 24);
	_spacer->resize(10, _min_h-44, 1, 1);
	_dialog->size_range(_min_w, _min_h, _min_w, _min_h);
	_dialog->size(_min_w, _min_h);
	_dialog->redraw();
}

Interpolation_Dialog::Interpolation_Dialog(const char *t) : Modal_Dialog(t, OK_CANCEL_DIALOG), _mdbu(NULL),
	_I_spinner(NULL), _md(NULL), _H_spinner(NULL), _rt_spinner(NULL), _rs_spinner(NULL) {
	min_size(_min_w, 154);
}

Interpolation_Dialog::~Interpolation_Dialog() {
	delete _mdbu;
	delete _I_spinner;
	delete _md;
	delete _H_spinner;
	delete _rt_spinner;
	delete _rs_spinner;
}

void Interpolation_Dialog::on_initialize() {
	_mdbu = new Fl_Check_Button(0, 0, 0, 0, "MDBU:");
	_I_spinner = new Fl_Spinner(0, 0, 0, 0, "I:");
	_md = new Fl_Check_Button(0, 0, 0, 0, "Diamond-square MD:");
	_H_spinner = new Fl_Spinner(0, 0, 0, 0, "H:");
	_rt_spinner = new Fl_Spinner(0, 0, 0, 0, "rt:");
	_rs_spinner = new Fl_Spinner(0, 0, 0, 0, "rs:");
	// Initialize parameter controls
	_mdbu->labelfont(OS_FONT);
	_mdbu->labelsize(OS_FONT_SIZE);
	_mdbu->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	_mdbu->set();
	_I_spinner->labelfont(OS_FONT);
	_I_spinner->labelsize(OS_FONT_SIZE);
	_I_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_I_spinner->textfont(OS_FONT);
	_I_spinner->textsize(OS_FONT_SIZE);
	_I_spinner->type(FL_FLOAT_INPUT);
	_I_spinner->range(-2.0, 2.0);
	_I_spinner->step(0.05);
	_I_spinner->value(0.4);
	_md->labelfont(OS_FONT);
	_md->labelsize(OS_FONT_SIZE);
	_md->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	_md->set();
	_H_spinner->labelfont(OS_FONT);
	_H_spinner->labelsize(OS_FONT_SIZE);
	_H_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_H_spinner->textfont(OS_FONT);
	_H_spinner->textsize(OS_FONT_SIZE);
	_H_spinner->type(FL_FLOAT_INPUT);
	_H_spinner->range(0.0, 2.0);
	_H_spinner->step(0.05);
	_H_spinner->value(1.0);
	_rt_spinner->labelfont(OS_FONT);
	_rt_spinner->labelsize(OS_FONT_SIZE);
	_rt_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_rt_spinner->textfont(OS_FONT);
	_rt_spinner->textsize(OS_FONT_SIZE);
	_rt_spinner->type(FL_FLOAT_INPUT);
	_rt_spinner->range(-1.0, 1.0);
	_rt_spinner->step(0.05);
	_rt_spinner->value(0.0);
	_rs_spinner->labelfont(OS_FONT);
	_rs_spinner->labelsize(OS_FONT_SIZE);
	_rs_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_rs_spinner->textfont(OS_FONT);
	_rs_spinner->textsize(OS_FONT_SIZE);
	_rs_spinner->type(FL_FLOAT_INPUT);
	_rs_spinner->range(-1.0, 1.0);
	_rs_spinner->step(0.05);
	_rs_spinner->value(1.0);
}

void Interpolation_Dialog::refresh() {
	// Refresh widget labels
	_dialog->label(_title);
	// Refresh widget positions and sizes
	_mdbu->resize(10, 10, 100, 22);
	_I_spinner->resize(30, 36, 48, 22);
	_md->resize(10, 62, 100, 22);
	_H_spinner->resize(30, 88, 48, 22);
	_rt_spinner->resize(108, 88, 48, 22);
	_rs_spinner->resize(186, 88, 48, 22);
	_min_h = 154;
	_ok_button->resize(_min_w-180, _min_h-34, 80, 24);
	_cancel_button->resize(_min_w-90, _min_h-34, 80, 24);
	_spacer->resize(9, _min_h-44, 1, 1);
	_dialog->size_range(_min_w, _min_h, _min_w, _min_h);
	_dialog->size(_min_w, _min_h);
	_dialog->redraw();
}

Erosion_Dialog::Erosion_Dialog(const char *t) : Modal_Dialog(t, OK_CANCEL_DIALOG), _time_step_spinner(NULL),
	_thermal(NULL), _Kt_spinner(NULL), _Ka_spinner(NULL), _Ki_spinner(NULL), _hydraulic(NULL), _Kc_spinner(NULL),
	_Kd_spinner(NULL), _Ks_spinner(NULL), _Ke_spinner(NULL), _W0_spinner(NULL), _Wmin_spinner(NULL) {
	min_size(_min_w, 206);
}

Erosion_Dialog::~Erosion_Dialog() {
	delete _time_step_spinner;
	delete _thermal;
	delete _Kt_spinner;
	delete _Ka_spinner;
	delete _Ki_spinner;
	delete _hydraulic;
	delete _Kc_spinner;
	delete _Kd_spinner;
	delete _Ks_spinner;
	delete _Ke_spinner;
	delete _W0_spinner;
	delete _Wmin_spinner;
}

void Erosion_Dialog::on_initialize() {
	_time_step_spinner = new Fl_Spinner(0, 0, 0, 0, "Time steps:");
	_thermal = new Fl_Check_Button(0, 0, 0, 0, "Thermal erosion:");
	_Kt_spinner = new Fl_Spinner(0, 0, 0, 0, "Kt:");
	_Ka_spinner = new Fl_Spinner(0, 0, 0, 0, "Ka:");
	_Ki_spinner = new Fl_Spinner(0, 0, 0, 0, "Ki:");
	_hydraulic = new Fl_Check_Button(0, 0, 0, 0, "Hydraulic erosion:");
	_Kc_spinner = new Fl_Spinner(0, 0, 0, 0, "Kc:");
	_Kd_spinner = new Fl_Spinner(0, 0, 0, 0, "Kd:");
	_Ks_spinner = new Fl_Spinner(0, 0, 0, 0, "Ks:");
	_Ke_spinner = new Fl_Spinner(0, 0, 0, 0, "Ke:");
	_W0_spinner = new Fl_Spinner(0, 0, 0, 0, "W0:");
	_Wmin_spinner = new Fl_Spinner(0, 0, 0, 0, "Wmin:");
	// Initialize parameter controls
	_time_step_spinner->labelfont(OS_FONT);
	_time_step_spinner->labelsize(OS_FONT_SIZE);
	_time_step_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_time_step_spinner->textfont(OS_FONT);
	_time_step_spinner->textsize(OS_FONT_SIZE);
	_time_step_spinner->type(FL_INT_INPUT);
	_time_step_spinner->range(1.0, 2000.0);
	_time_step_spinner->step(1.0);
	_time_step_spinner->value(100.0);
	_thermal->labelfont(OS_FONT);
	_thermal->labelsize(OS_FONT_SIZE);
	_thermal->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	_thermal->set();
	_Kt_spinner->labelfont(OS_FONT);
	_Kt_spinner->labelsize(OS_FONT_SIZE);
	_Kt_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_Kt_spinner->textfont(OS_FONT);
	_Kt_spinner->textsize(OS_FONT_SIZE);
	_Kt_spinner->type(FL_FLOAT_INPUT);
	_Kt_spinner->range(0.0, 3.0);
	_Kt_spinner->step(0.05);
	_Kt_spinner->value(0.15);
	_Ka_spinner->labelfont(OS_FONT);
	_Ka_spinner->labelsize(OS_FONT_SIZE);
	_Ka_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_Ka_spinner->textfont(OS_FONT);
	_Ka_spinner->textsize(OS_FONT_SIZE);
	_Ka_spinner->type(FL_FLOAT_INPUT);
	_Ka_spinner->range(0.0, 1.57);
	_Ka_spinner->step(0.05);
	_Ka_spinner->value(0.8);
	_Ki_spinner->labelfont(OS_FONT);
	_Ki_spinner->labelsize(OS_FONT_SIZE);
	_Ki_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_Ki_spinner->textfont(OS_FONT);
	_Ki_spinner->textsize(OS_FONT_SIZE);
	_Ki_spinner->type(FL_FLOAT_INPUT);
	_Ki_spinner->range(0.0, 1.57);
	_Ki_spinner->step(0.05);
	_Ki_spinner->value(0.1);
	_hydraulic->labelfont(OS_FONT);
	_hydraulic->labelsize(OS_FONT_SIZE);
	_hydraulic->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	_hydraulic->set();
	_Kc_spinner->labelfont(OS_FONT);
	_Kc_spinner->labelsize(OS_FONT_SIZE);
	_Kc_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_Kc_spinner->textfont(OS_FONT);
	_Kc_spinner->textsize(OS_FONT_SIZE);
	_Kc_spinner->type(FL_INT_INPUT);
	_Kc_spinner->range(1.0, 512.0);
	_Kc_spinner->step(1.0);
	_Kc_spinner->value(8.0);
	_Kd_spinner->labelfont(OS_FONT);
	_Kd_spinner->labelsize(OS_FONT_SIZE);
	_Kd_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_Kd_spinner->textfont(OS_FONT);
	_Kd_spinner->textsize(OS_FONT_SIZE);
	_Kd_spinner->type(FL_FLOAT_INPUT);
	_Kd_spinner->range(0.0, 1.0);
	_Kd_spinner->step(0.01);
	_Kd_spinner->value(0.05);
	_Ks_spinner->labelfont(OS_FONT);
	_Ks_spinner->labelsize(OS_FONT_SIZE);
	_Ks_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_Ks_spinner->textfont(OS_FONT);
	_Ks_spinner->textsize(OS_FONT_SIZE);
	_Ks_spinner->type(FL_FLOAT_INPUT);
	_Ks_spinner->range(0.0, 1.0);
	_Ks_spinner->step(0.01);
	_Ks_spinner->value(0.1);
	_Ke_spinner->labelfont(OS_FONT);
	_Ke_spinner->labelsize(OS_FONT_SIZE);
	_Ke_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_Ke_spinner->textfont(OS_FONT);
	_Ke_spinner->textsize(OS_FONT_SIZE);
	_Ke_spinner->type(FL_FLOAT_INPUT);
	_Ke_spinner->range(0.0, 1.0);
	_Ke_spinner->step(0.01);
	_Ke_spinner->value(0.01);
	_W0_spinner->labelfont(OS_FONT);
	_W0_spinner->labelsize(OS_FONT_SIZE);
	_W0_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_W0_spinner->textfont(OS_FONT);
	_W0_spinner->textsize(OS_FONT_SIZE);
	_W0_spinner->type(FL_FLOAT_INPUT);
	_W0_spinner->range(0.0, 4.0);
	_W0_spinner->step(0.05);
	_W0_spinner->value(1.0);
	_Wmin_spinner->labelfont(OS_FONT);
	_Wmin_spinner->labelsize(OS_FONT_SIZE);
	_Wmin_spinner->align(FL_ALIGN_LEFT | FL_ALIGN_CLIP);
	_Wmin_spinner->textfont(OS_FONT);
	_Wmin_spinner->textsize(OS_FONT_SIZE);
	_Wmin_spinner->type(FL_FLOAT_INPUT);
	_Wmin_spinner->range(0.0, 1.0);
	_Wmin_spinner->step(0.01);
	_Wmin_spinner->value(0.01);
}

void Erosion_Dialog::refresh() {
	// Refresh widget labels
	_dialog->label(_title);
	// Refresh widget positions and sizes
	_time_step_spinner->resize(75, 10, 48, 22);
	_thermal->resize(10, 36, 150, 22);
	_Kt_spinner->resize(30, 62, 48, 22);
	_Ka_spinner->resize(108, 62, 48, 22);
	_Ki_spinner->resize(186, 62, 48, 22);
	_hydraulic->resize(10, 88, 150, 22);
	_Kc_spinner->resize(30, 114, 48, 22);
	_Kd_spinner->resize(108, 114, 48, 22);
	_Ks_spinner->resize(186, 114, 48, 22);
	_Ke_spinner->resize(30, 140, 48, 22);
	_W0_spinner->resize(113, 140, 48, 22);
	_Wmin_spinner->resize(211, 140, 48, 22);
	_min_h = 206;
	_ok_button->resize(_min_w-180, _min_h-34, 80, 24);
	_cancel_button->resize(_min_w-90, _min_h-34, 80, 24);
	_spacer->resize(9, _min_h-44, 1, 1);
	_dialog->size_range(_min_w, _min_h, _min_w, _min_h);
	_dialog->size(_min_w, _min_h);
	_dialog->redraw();
}
