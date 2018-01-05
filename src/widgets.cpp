#include <iostream>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Spinner.H>
#include <FL/Fl_Hor_Nice_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/fl_draw.H>
#pragma warning(pop)

#include "os-font.h"
#include "widgets.h"

Fl_Spacer::Fl_Spacer(int x, int y, int w, int h, const char *l) : Fl_Box(x, y, w, h, l) {
	labeltype(FL_NO_LABEL);
	box(FL_THIN_DOWN_FRAME);
}

Fl_Text::Fl_Text(int x, int y, int w, int h, const char *l) : Fl_Box(x, y, w, h, l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
}

Fl_Default_Spinner::Fl_Default_Spinner(int x, int y, int w, int h, const char *l) : Fl_Spinner(x, y, w, h, l),
	_default_value(0.0) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
}

int Fl_Default_Spinner::handle(int event) {
	switch (event) {
	case FL_PUSH:
		if (Fl::event_button() == FL_MIDDLE_MOUSE) {
			return 1;
		}
		break;
	case FL_RELEASE:
		if (Fl::event_button() == FL_MIDDLE_MOUSE) {
			value(_default_value);
			do_callback();
			return 1;
		}
		break;
	}
	return Fl_Spinner::handle(event);
}

Fl_Default_Slider::Fl_Default_Slider(int x, int y, int w, int h, const char *l) : Fl_Hor_Nice_Slider(x, y, w, h, l),
	_default_value(0.0) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
}

int Fl_Default_Slider::handle(int event) {
	switch (event) {
	case FL_PUSH:
		if (Fl::event_button() == FL_MIDDLE_MOUSE) {
			return 1;
		}
		break;
	case FL_RELEASE:
		if (Fl::event_button() == FL_MIDDLE_MOUSE) {
			value(_default_value);
			do_callback();
			return 1;
		}
		break;
	}
	return Fl_Hor_Nice_Slider::handle(event);
}

Fl_Control_Group::Fl_Control_Group(int x, int y, int w, int h, const char *l) : Fl_Group(x, y, w, h, l) {
	labeltype(FL_NO_LABEL);
	box(FL_DOWN_FRAME);
}

Fl_Sidebar::Fl_Sidebar(int x, int y, int w, int h, const char *l) : Fl_Group(x, y, w, h, l), _spacer(x, y + h, w, 0) {
	labeltype(FL_NO_LABEL);
	box(FL_UP_FRAME);
	resizable(_spacer);
}

Fl_Toolbar::Fl_Toolbar(int x, int y, int w, int h, const char *l) : Fl_Pack(x, y, w, h, l), _spacer(0, 0, 0, 0) {
	type(HORIZONTAL);
	labeltype(FL_NO_LABEL);
	box(FL_UP_FRAME);
	resizable(_spacer);
}

void Fl_Toolbar::draw() {
	// Keep the resizable spacer as the last child
	insert(_spacer, children());
	resizable(_spacer);
	Fl_Pack::draw();
}

Fl_Toolbar_Button::Fl_Toolbar_Button(int x, int y, int w, int h, const char *l) : Fl_Button(x, y, w, h, l) {
	box(FL_FLAT_BOX);
	down_box(FL_DOWN_BOX);
	down_color(FL_DARK2);
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	align(FL_ALIGN_IMAGE_NEXT_TO_TEXT | FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
}

int Fl_Toolbar_Button::handle(int event) {
	switch (event) {
	case FL_ENTER:
		color(FL_LIGHT3);
		redraw();
		return 1;
	case FL_LEAVE:
		color(FL_BACKGROUND_COLOR);
		redraw();
		return 1;
	}
	return Fl_Button::handle(event);
}

Fl_Toolbar_Toggle_Button::Fl_Toolbar_Toggle_Button(int x, int y, int w, int h, const char *l) :
	Fl_Toolbar_Button(x, y, w, h, l) {
	type(FL_TOGGLE_BUTTON);
}

Fl_Toolbar_Radio_Button::Fl_Toolbar_Radio_Button(int x, int y, int w, int h, const char *l) :
	Fl_Toolbar_Button(x, y, w, h, l) {
	type(FL_RADIO_BUTTON);
}

static Fl_Toolbar_Dropdown_Button *pressed_dropdown_tb = NULL;

Fl_Toolbar_Dropdown_Button::Fl_Toolbar_Dropdown_Button(int x, int y, int w, int h, const char *l) :
	Fl_Menu_(x, y, w, h, l) {
	box(FL_FLAT_BOX);
	down_box(FL_FLAT_BOX);
	down_color(FL_SELECTION_COLOR);
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
}

const Fl_Menu_Item *Fl_Toolbar_Dropdown_Button::dropdown() {
	// Based on Fl_Menu_Button::popup(...)
	pressed_dropdown_tb = this;
	redraw();
	Fl_Widget_Tracker mb(this);
	const Fl_Menu_Item *m = menu()->pulldown(x(), y(), w(), h(), NULL, this);
	picked(m);
	pressed_dropdown_tb = NULL;
	if (mb.exists()) { redraw(); }
	return m;
}

void Fl_Toolbar_Dropdown_Button::draw() {
	// Based on Fl_Button::draw()
	bool pressed = pressed_dropdown_tb == this;
	Fl_Color col = pressed ? selection_color() : color();
	draw_box(pressed ? down_box() ? down_box() : fl_down(box()) : box(), col);
	draw_backdrop();
	if (labeltype() == FL_NORMAL_LABEL && pressed) {
		Fl_Color c = labelcolor();
		labelcolor(fl_contrast(c, col));
		draw_label();
		labelcolor(c);
	}
	else {
		draw_label();
	}
	if (Fl::focus() == this) { draw_focus(); }
}

int Fl_Toolbar_Dropdown_Button::handle(int event) {
	// Based on Fl_Menu_Button::handle(int e)
	if (!menu() || !menu()->text) { return 0; }
	switch (event) {
	case FL_ENTER:
		color(FL_LIGHT3);
		redraw();
		return box() ? 1 : 0;
	case FL_LEAVE:
		color(FL_BACKGROUND_COLOR);
		redraw();
		return box() ? 1 : 0;
	case FL_PUSH:
		if (!box()) {
			if (Fl::event_button() != FL_RIGHT_MOUSE) { return 0; }
		}
		if (Fl::visible_focus()) { Fl::focus(this); }
		color(FL_BACKGROUND_COLOR);
		dropdown();
		return 1;
	case FL_KEYBOARD:
		if (!box()) { return 0; }
		if (Fl::event_key() == ' ' && !(Fl::event_state() & (FL_SHIFT | FL_CTRL | FL_ALT | FL_META))) {
			dropdown();
			return 1;
		}
		return 0;
	case FL_SHORTCUT:
		if (Fl_Widget::test_shortcut()) {
			dropdown();
			return 1;
		}
		return test_shortcut() != 0;
	case FL_FOCUS:
	case FL_UNFOCUS:
		if (box() && Fl::visible_focus()) {
			redraw();
			return 1;
		}
	}
	return 0;
}

Fl_Status_Bar_Field::Fl_Status_Bar_Field(int x, int y, int w, int h, const char *l) : Fl_Box(x, y, w, h, l),
	_default_label(l) {
	labelfont(OS_FONT);
	labelsize(OS_FONT_SIZE);
	align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
}
