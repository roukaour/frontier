#pragma once

#include <cstdlib>

#pragma warning(push, 0)
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Hor_Nice_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Menu_.H>
#pragma warning(pop)

class Fl_Spacer : public Fl_Box {
public:
	Fl_Spacer(int x, int y, int w, int h, const char *l = NULL);
};

class Fl_Text : public Fl_Box {
public:
	Fl_Text(int x, int y, int w, int h, const char *l = NULL);
};

class Fl_Default_Spinner : public Fl_Spinner {
private:
	double _default_value;
public:
	Fl_Default_Spinner(int x, int y, int w, int h, const char *l = NULL);
	inline double default_value(void) const { return _default_value; }
	inline void default_value(double v) { _default_value = v; value(_default_value); }
protected:
	int handle(int event);
};

class Fl_Default_Slider : public Fl_Hor_Nice_Slider {
private:
	double _default_value;
public:
	Fl_Default_Slider(int x, int y, int w, int h, const char *l = NULL);
	inline double default_value(void) const { return _default_value; }
	inline void default_value(double v) { _default_value = v; value(_default_value); }
protected:
	int handle(int event);
};

class Fl_Control_Group : public Fl_Group {
public:
	Fl_Control_Group(int x, int y, int w, int h, const char *l = NULL);
};

class Fl_Sidebar : public Fl_Group {
private:
	Fl_Box _spacer;
public:
	Fl_Sidebar(int x, int y, int w, int h, const char *l = NULL);
};

class Fl_Toolbar : public Fl_Pack {
private:
	Fl_Box _spacer;
public:
	Fl_Toolbar(int x, int y, int w, int h, const char *l = NULL);
protected:
	void draw(void);
};

class Fl_Toolbar_Button : public Fl_Button {
public:
	Fl_Toolbar_Button(int x, int y, int w, int h, const char *l = NULL);
protected:
	int handle(int event);
};

class Fl_Toolbar_Toggle_Button : public Fl_Toolbar_Button {
public:
	Fl_Toolbar_Toggle_Button(int x, int y, int w, int h, const char *l = NULL);
};

class Fl_Toolbar_Radio_Button : public Fl_Toolbar_Button {
public:
	Fl_Toolbar_Radio_Button(int x, int y, int w, int h, const char *l = NULL);
};

class Fl_Toolbar_Dropdown_Button : public Fl_Menu_ {
public:
	Fl_Toolbar_Dropdown_Button(int x, int y, int w, int h, const char *l = NULL);
	const Fl_Menu_Item *dropdown(void);
protected:
	void draw(void);
	int handle(int event);
};

class Fl_Status_Bar_Field : public Fl_Box {
private:
	const char *_default_label;
public:
	Fl_Status_Bar_Field(int x, int y, int w, int h, const char *l = NULL);
	inline const char *default_label(void) const { return _default_label; }
	inline void reset_label(void) { copy_label(_default_label); }
};
