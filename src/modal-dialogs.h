#pragma once

#include <cstdlib>

#pragma warning(push, 0)
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/fl_draw.H>
#pragma warning(pop)

#include "os-font.h"
#include "widgets.h"

#define PROGRESS_STEPS 100

class Modal_Dialog {
public:
	enum Dialog_Type { OK_DIALOG = 0x1, CANCEL_DIALOG = 0x2, OK_CANCEL_DIALOG = 0x1 | 0x2 };
protected:
	Dialog_Type _dialog_type;
	const char *_title;
	int _min_w, _min_h;
	bool _canceled;
	Fl_Double_Window *_dialog;
	Fl_Button *_ok_button, *_cancel_button;
	Fl_Box *_spacer;
public:
	Modal_Dialog(const char *t = NULL, Dialog_Type dt = OK_DIALOG);
	virtual ~Modal_Dialog();
protected:
	void initialize(void);
	virtual void on_initialize(void) {}
	virtual void refresh(void) = 0;
	virtual void on_show(const Fl_Widget *) {}
	virtual void on_hide(void) {}
public:
	inline void title(const char *t, bool del = false) { if (del) { delete _title; } _title = t; }
	inline void min_size(int w, int h) { _min_w = w; _min_h = h; }
	inline bool canceled(void) const { return _canceled; }
	inline void canceled(bool c) { _canceled = c; }
	void show(const Fl_Widget *p, bool wait);
	void hide(void) { _dialog->hide(); on_hide(); }
private:
	static void close_cb(Fl_Widget *, Modal_Dialog *md);
	static void cancel_cb(Fl_Widget *w, Modal_Dialog *md);
};

class Alert_Dialog : public Modal_Dialog {
public:
	enum Icon_Type { NO_ICON, INFO_ICON, QUESTION_ICON, WARNING_ICON, ERROR_ICON, SUCCESS_ICON, PROGRAM_ICON };
private:
	Icon_Type _icon_type;
	const char *_subject;
	const char *_message;
	bool _wrap_body;
	Fl_Box *_icon;
	Fl_Box *_heading;
	Fl_Box *_body;
public:
	Alert_Dialog(const char *t = NULL, Icon_Type it = NO_ICON);
	~Alert_Dialog();
protected:
	void on_initialize(void);
	void refresh(void);
public:
	inline void icon(Icon_Type it) { _icon_type = it; }
	inline void subject(const char *s, bool del = false) { if (del) { delete _subject; } _subject = s; }
	inline void message(const char *m, bool del = false) { if (del) { delete _message; } _message = m; }
	inline void wrap_body(bool w) { _wrap_body = w; }
	void show(const Fl_Widget *p) { Modal_Dialog::show(p, true); }
};

class Progress_Dialog : public Modal_Dialog {
private:
	Fl_Box *_body;
	Fl_Progress *_progress;
public:
	Progress_Dialog(const char *t = NULL);
	~Progress_Dialog();
protected:
	void on_initialize(void);
	void refresh(void);
	void on_show(const Fl_Widget *) { fl_cursor(FL_CURSOR_WAIT); }
	void on_hide(void) { fl_cursor(FL_CURSOR_DEFAULT); }
public:
	inline void message(const char *m) { _body->copy_label(m); }
	inline void progress(float p) { _progress->value(p); }
	void show(const Fl_Widget *p) { Modal_Dialog::show(p, false); }
};

class New_DTED_Dialog : public Modal_Dialog {
private:
	Fl_Spinner *_width_spinner, *_height_spinner;
	Fl_Text *_width_spinner_units, *_height_spinner_units;
public:
	New_DTED_Dialog(const char *t = NULL);
	~New_DTED_Dialog();
protected:
	void on_initialize(void);
	void refresh(void);
public:
	inline size_t dted_width(void) const { return (size_t)_width_spinner->value(); }
	inline void dted_width(size_t w) { _width_spinner->value((double)w); }
	inline size_t dted_height(void) const { return (size_t)_height_spinner->value(); }
	inline void dted_height(size_t h) { _height_spinner->value((double)h); }
	void show(const Fl_Widget *p) { Modal_Dialog::show(p, true); }
};

class Decimation_Dialog : public Modal_Dialog {
private:
	Fl_Text *_keep_label;
	Fl_Radio_Round_Button *_keep_random, *_keep_edges;
	Fl_Spinner *_percent_spinner;
	Fl_Text *_percent_spinner_units;
public:
	Decimation_Dialog(const char *t = NULL);
	~Decimation_Dialog();
protected:
	void on_initialize(void);
	void refresh(void);
public:
	inline bool keep_random(void) const { return _keep_random->value() != 0.0; }
	inline void keep_random(bool r) { (r ? _keep_random : _keep_edges)->setonly(); }
	inline double decimation_threshold(void) const { return _percent_spinner->value() / 100.0; }
	inline void decimation_threshold(double p) { _percent_spinner->value(p * 100.0); }
	void show(const Fl_Widget *p) { Modal_Dialog::show(p, true); }
};

class Expansion_Dialog : public Modal_Dialog {
private:
	Fl_Spinner *_power_spinner;
public:
	Expansion_Dialog(const char *t = NULL);
	~Expansion_Dialog();
protected:
	void on_initialize(void);
	void refresh(void);
public:
	inline size_t expansion_power(void) const { return (size_t)_power_spinner->value(); }
	inline void expansion_power(size_t p) { _power_spinner->value((double)p); }
	void show(const Fl_Widget *p) { Modal_Dialog::show(p, true); }
};

class Interpolation_Dialog : public Modal_Dialog {
private:
	Fl_Check_Button *_mdbu;
	Fl_Spinner *_I_spinner;
	Fl_Check_Button *_md;
	Fl_Spinner *_H_spinner, *_rt_spinner, *_rs_spinner;
public:
	Interpolation_Dialog(const char *t = NULL);
	~Interpolation_Dialog();
protected:
	void on_initialize(void);
	void refresh(void);
public:
	inline bool mdbu_process(void) const { return _mdbu->value() != 0.0; }
	inline void mdbu_process(bool m) { if (m) { _mdbu->set(); } else { _mdbu->clear(); } }
	inline float param_I(void) const { return (float)_I_spinner->value(); }
	inline void param_I(float I) { _I_spinner->value((double)I); }
	inline bool md_process(void) const { return _md->value() != 0.0; }
	inline void md_process(bool m) { if (m) { _md->set(); } else { _md->clear(); } }
	inline float param_H(void) const { return (float)_H_spinner->value(); }
	inline void param_H(float H) { _H_spinner->value((double)H); }
	inline float param_rt(void) const { return (float)_rt_spinner->value(); }
	inline void param_rt(float rt) { _rt_spinner->value((double)rt); }
	inline float param_rs(void) const { return (float)_rs_spinner->value(); }
	inline void param_rs(float rs) { _rs_spinner->value((double)rs); }
	void show(const Fl_Widget *p) { Modal_Dialog::show(p, true); }
};

class Erosion_Dialog : public Modal_Dialog {
private:
	Fl_Spinner *_time_step_spinner; // number of time steps (1-1000)
	Fl_Check_Button *_thermal;
	Fl_Spinner *_Kt_spinner; // thermal erosion rate (0-3)
	Fl_Spinner *_Ka_spinner; // talus angle tangent coefficient (0-1)
	Fl_Spinner *_Ki_spinner; // talus angle tangent bias (0-1)
	Fl_Check_Button *_hydraulic;
	Fl_Spinner *_Kc_spinner; // maximum amount of sediment which can be suspended in a unit of water (1-512) [8]
	Fl_Spinner *_Kd_spinner; // sediment deposition rate (0-1) [0.05]
	Fl_Spinner *_Ks_spinner; // sedimentation rate (0-1) [0.1]
	Fl_Spinner *_Ke_spinner; // evaporation rate for water (0-1) [0.01]
	Fl_Spinner *_W0_spinner; // maximum amount of rain per column (0-1) [1]
	Fl_Spinner *_Wmin_spinner; // minimum amount of rain per column (0-1) [0.01]
public:
	Erosion_Dialog(const char *t = NULL);
	~Erosion_Dialog();
protected:
	void on_initialize(void);
	void refresh(void);
public:
	inline size_t param_nts(void) const { return (size_t)_time_step_spinner->value(); }
	inline void param_nts(size_t nts) const { _time_step_spinner->value((double)nts); }
	inline bool thermal_erosion(void) const { return _thermal->value() != 0.0; }
	inline void thermal_erosion(bool e) { if (e) { _thermal->set(); } else { _thermal->clear(); } }
	inline float param_Kt(void) const { return (float)_Kt_spinner->value(); }
	inline void param_Kt(float Kt) { _Kt_spinner->value((double)Kt); }
	inline float param_Ka(void) const { return (float)_Ka_spinner->value(); }
	inline void param_Ka(float Ka) { _Ka_spinner->value((double)Ka); }
	inline float param_Ki(void) const { return (float)_Ki_spinner->value(); }
	inline void param_Ki(float Ki) { _Ki_spinner->value((double)Ki); }
	inline bool hydraulic_erosion(void) const { return _hydraulic->value() != 0.0; }
	inline void hydraulic_erosion(bool e) { if (e) { _hydraulic->set(); } else { _hydraulic->clear(); } }
	inline float param_Kc(void) const { return (float)_Ki_spinner->value(); }
	inline void param_Kc(float Kc) { _Kc_spinner->value((double)Kc); }
	inline float param_Kd(void) const { return (float)_Ki_spinner->value(); }
	inline void param_Kd(float Kd) { _Kd_spinner->value((double)Kd); }
	inline float param_Ks(void) const { return (float)_Ki_spinner->value(); }
	inline void param_Ks(float Ks) { _Ks_spinner->value((double)Ks); }
	inline float param_Ke(void) const { return (float)_Ki_spinner->value(); }
	inline void param_Ke(float Ke) { _Ke_spinner->value((double)Ke); }
	inline float param_W0(void) const { return (float)_W0_spinner->value(); }
	inline void param_W0(float W0) { _W0_spinner->value((double)W0); }
	inline float param_Wmin(void) const { return (float)_Wmin_spinner->value(); }
	inline void param_Wmin(float Wmin) { _Wmin_spinner->value((double)Wmin); }
	void show(const Fl_Widget *p) { Modal_Dialog::show(p, true); }
};
