#include <cstdlib>
#include <iostream>

#pragma warning(push, 0)
#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/glut.h>
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_PNG_Image.H>
#pragma warning(pop)

#include "algebra.h"
#include "draw-state.h"
#include "heightmap.h"
#include "workspace.h"

const double Workspace::FOV_Y = 45.0;

const double Workspace::NEAR_PLANE = 0.5;
const double Workspace::FAR_PLANE = 12.0;

const double Workspace::FOCAL_LENGTH = 5.0;

const double Workspace::PAN_SCALE = 2.25;
const double Workspace::ZOOM_SCALE = 1.5;

Workspace::Workspace(int x, int y, int w, int h) : Fl_Gl_Window(x, y, w, h, NULL), _initialized(false), _opened(false),
	_dragging(false), _left_mouse(false), _heightmap(), _state(), _prev_state(), _click_coords(), _drag_coords() {
	end();
}

bool Workspace::create(size_t w, size_t h) {
	close();
	_opened = _heightmap.create(w, h);
	redraw();
	return _opened;
}

bool Workspace::open(const char *filename) {
	close();
	_opened = _heightmap.open(filename);
	if (_state.render_3d()) { calculate_normals(); }
	redraw();
	return _opened;
}

bool Workspace::save(const char *filename, Progress_Dialog *pd) {
	return _heightmap.save(filename, _state.color_scheme(), pd);
}

void Workspace::close() {
	_heightmap.clear();
	_state.reset();
	_prev_state = _state;
	_opened = false;
	redraw();
}

void Workspace::rotate() {
	if (!_opened || !_state.render_3d()) { return; }
	double r = 1.0 / _state.zoom();
	double p1[2] = {2.0 * _click_coords[0] / w() - 1.0, 2.0 * _click_coords[1] / h() - 1.0};
	double p2[2] = {2.0 * _drag_coords[0] / w() - 1.0, 2.0 * _drag_coords[1] / h() - 1.0};
	if (w() > h()) { p1[0] *= (double)w() / h(); p2[0] *= (double)w() / h(); }
	else { p1[1] *= (double)h() / w(); p2[1] *= (double)h() / w(); }
	double dm[16], m[16];
	arcball_matrix(r, p1, p2, dm);
	matrix_mul(_prev_state.rotate(), dm, m);
	_state.rotate(m);
	redraw();
}

void Workspace::pan(int dx, int dy) {
	if (!_opened) { return; }
	if (_state.render_3d()) {
		double d[2] = {(double)dx / w(), (double)dy / h()};
		if (w() > h()) { d[0] *= (double)w() / h(); }
		else { d[1] *= (double)h() / w(); }
		_state.pan(_prev_state.pan_x() + PAN_SCALE * _state.zoom() * d[0],
			_prev_state.pan_y() + PAN_SCALE * _state.zoom() * d[1]);
	}
	else {
		_prev_state = _state;
		double sdx = (double)dx / w(), sdy = (double)dy / h();
		if (w() > h()) { sdx *= (double)w() / h(); }
		else { sdy *= (double)h() / w(); }
		double scale = (double)MIN(w(), h()) / _state.zoom();
		_state.pan(_prev_state.pan_x() + scale * sdx, _prev_state.pan_y() + scale * sdy);
	}
	redraw();
}

void Workspace::zoom_in(int cx, int cy) {
	if (!_opened) { return; }
	if (_state.render_3d()) {
		_prev_state = _state;
		_state.zoom(_prev_state.zoom() / CUBRT_2);
		invalidate();
	}
	else {
		_prev_state = _state;
		_state.zoom(_prev_state.zoom() * CUBRT_2, cx, cy);
	}
	redraw();
}

void Workspace::zoom_out(int cx, int cy) {
	if (!_opened) { return; }
	if (_state.render_3d()) {
		_prev_state = _state;
		_state.zoom(_prev_state.zoom() * CUBRT_2);
		invalidate();
	}
	else {
		_prev_state = _state;
		_state.zoom(_prev_state.zoom() / CUBRT_2, cx, cy);
	}
	redraw();
}

void Workspace::zoom_reset(int cx, int cy) {
	if (!_opened) { return; }
	if (_state.render_3d()) {
		_prev_state = _state;
		_state.zoom(1.0);
	}
	else {
		_prev_state = _state;
		_state.zoom(1.0, cx, cy);
	}
	redraw();
}

void Workspace::decimate(bool random, double thresh, Progress_Dialog *pd) {
	if (!_opened) { return; }
	_heightmap.decimate(random, thresh, pd);
	redraw();
}

bool Workspace::expand(size_t power, Progress_Dialog *pd) {
	if (!_opened) { return true; }
	bool success = _heightmap.expand(power, pd);
	redraw();
	return success;
}

void Workspace::interpolate(bool mdbu, float I, bool md, float H, float rt, float rs, Progress_Dialog *pd) {
	if (!_opened) { return; }
	_heightmap.interpolate(mdbu, I, md, H, rt, rs, pd);
	if (_state.render_3d()) { calculate_normals(pd); }
	redraw();
}

void Workspace::erode(size_t nts, bool thermal, float Kt, float Ka, float Ki, bool hydraulic, float Kc, float Kd,
	float Ks, float Ke, float W0, float Wmin, Progress_Dialog *pd) {
	if (!_opened) { return; }
	_heightmap.erode(nts, thermal, Kt, Ka, Ki, hydraulic, Kc, Kd, Ks, Ke, W0, Wmin, pd);
	if (_state.render_3d()) { calculate_normals(pd); }
	redraw();
}

bool Workspace::calculate_normals(Progress_Dialog *pd) {
	if (!_opened) { return true; }
	bool success = _heightmap.calculate_normals(pd);
	invalidate();
	redraw();
	return success;
}

void Workspace::render_3d(bool r) {
	_state.reset();
	_state.render_3d(r);
	invalidate();
	redraw();
}

void Workspace::refresh_gl() {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gl_font(FL_SCREEN, 12);
}

void Workspace::refresh_projection() const {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0);
	glViewport(0, 0, w(), h());
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (_state.render_3d()) {
		double aspect = (double)w() / h();
		double r = MAX(_heightmap.width(), _heightmap.height()) / 2.0;
		gluPerspective(FOV_Y * _state.zoom(), aspect, NEAR_PLANE * r, FAR_PLANE * r);
		glScaled(1.0, -1.0, 1.0); // flip upside-down
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		float ambient[4] = {0.125f, 0.125f, 0.125f, 1.0f};
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		float diffuse[4] = {0.8125f, 0.8125f, 0.8125f, 1.0f};
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		float specular[4] = {0.0f, 0.0f, 0.0f, 1.0f};
		glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
		float global_ambient[4] = {0.125f, 0.125f, 0.125f, 1.0f};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
	}
	else {
		glOrtho(0.0, w(), h(), 0.0, -1.0, 1.0);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}
}

void Workspace::refresh_view() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (_state.render_3d()) {
		double c[3] = {_heightmap.width() / 2.0, _heightmap.height() / 2.0, 0.5 * _state.scale()};
		double r = MAX(_heightmap.width(), _heightmap.height()) / 2.0;
		gluLookAt(c[0], c[1], c[2] + FOCAL_LENGTH * r, c[0], c[1], c[2], 0.0, 1.0, 0.0);
		double px = _state.pan_x() * r, py = _state.pan_y() * r;
		glTranslated(px, py, 0.0);
		glTranslated(c[0], c[1], c[2]);
		glMultMatrixd(_state.rotate());
		glTranslated(-c[0], -c[1], -c[2]);
		float position[4] = {(float)(c[0] - px), (float)(c[1] - py), 0.25f * _state.scale(), 1.0f};
		glLightfv(GL_LIGHT0, GL_POSITION, position);
	}
	else {
		double zoom = _state.zoom();
		glScaled(zoom, zoom, 1.0);
		glTranslated((w() - (signed int)_heightmap.width()) / 2.0,
			(h() - (signed int)_heightmap.height()) / 2.0, 0.0); // center heightmap
		glTranslated(_state.pan_x(), _state.pan_y(), 0.0);
	}
}

void Workspace::draw() {
	if (!_initialized) {
		refresh_gl();
		_initialized = true;
	}
	if (!valid()) {
		refresh_projection();
		valid(1);
	}
	refresh_view();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//gl_draw(" ", 1); // fix for erratic FLTK font drawing <http://www.fltk.org/newsgroups.php?gfltk.opengl+v:17>
	if (_opened) {
		if (_state.render_3d()) {
			draw_heightmap_3d();
		}
		else {
			draw_heightmap_2d();
		}
	}
}

void Workspace::draw_heightmap_2d() {
	double zoom = _state.zoom();
	glPointSize(MAX((float)ceil(zoom), 1.0f));
	size_t ww = _heightmap.width(), hh = _heightmap.height();
	size_t min_x = (size_t)MAX(0.0 - _state.pan_x() - (w() - (signed int)ww) / 2.0 - 1, 0.0);
	size_t max_x = (size_t)MIN(w()/zoom - _state.pan_x() - (w() - (signed int)ww) / 2.0, ww - 1.0);
	size_t min_y = (size_t)MAX(0.0 - _state.pan_y() - (h() - (signed int)hh) / 2.0 - 1, 0.0);
	size_t max_y = (size_t)MIN(h()/zoom - _state.pan_y() - (h() - (signed int)hh) / 2.0, hh - 1.0);
	Color_Scheme cs = _state.color_scheme();
	// Draw points
	glBegin(GL_POINTS);
	for (size_t y = min_y; y <= max_y; y++) {
		for (size_t x = min_x; x <= max_x; x++) {
			Column &c = _heightmap.column(x, y);
			float h = c.elevation;
			if (h == Heightmap::UNKNOWN_ELEVATION) { continue; }
			float cv[3];
			column_color(c, cs, cv);
			glColor3fv(cv);
			glVertex3i((int)x, (int)y, 0);
		}
	}
	glEnd();
	// Draw box around points
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_LOOP);
	glVertex3d(-0.5, -0.5, 0.0);
	glVertex3d(ww - 0.5, -0.5, 0.0);
	glVertex3d(ww - 0.5, hh - 0.5, 0.0);
	glVertex3d(-0.5, hh - 0.5, 0.0);
	glEnd();
}

static void color_floor(float *cv) {
	float floor = 0.1875f;
	if (cv[0] < floor) { cv[0] = floor; }
	if (cv[1] < floor) { cv[1] = floor; }
	if (cv[2] < floor) { cv[2] = floor; }
}

void Workspace::draw_heightmap_3d() {
	size_t ww = _heightmap.width(), hh = _heightmap.height();
	Color_Scheme cs = _state.color_scheme();
	for (size_t y = 0; y < hh - 1; y++) {
		glBegin(GL_TRIANGLE_STRIP);
		for (size_t x = 0; x < ww; x++) {
			Column &c1 = _heightmap.column(x, y);
			float h1 = c1.elevation * _state.scale();
			float cv1[3];
			column_color(c1, cs, cv1);
			color_floor(cv1);
			glColor3fv(cv1);
			glNormal3fv(c1.normal.xyz);
			glVertex3f((float)x, (float)y, h1);
			Column &c2 = _heightmap.column(x, y + 1);
			float h2 = c2.elevation * _state.scale();
			float cv2[3];
			column_color(c2, cs, cv2);
			color_floor(cv2);
			glColor3fv(cv2);
			glNormal3fv(c2.normal.xyz);
			glVertex3f((float)x, (float)(y + 1), h2);
		}
		glEnd();
	}
}

int Workspace::handle(int event) {
	switch (event) {
	case FL_ENTER:
		fl_cursor(FL_CURSOR_MOVE);
		return 1;
	case FL_LEAVE:
		fl_cursor(FL_CURSOR_DEFAULT);
		return 1;
	}
	return _state.render_3d() ? handle_3d(event) : handle_2d(event);
}

int Workspace::handle_2d(int event) {
	switch (event) {
	case FL_PUSH:
		// start panning
		_click_coords[0] = Fl::event_x() - x(); _click_coords[1] = Fl::event_y() - y();
		_dragging = true;
		_prev_state = _state;
		return 1;
	case FL_RELEASE:
		// stop panning
		_drag_coords[0] = Fl::event_x() - x(); _drag_coords[1] = Fl::event_y() - y();
		_dragging = false;
		return 1;
	case FL_DRAG:
		// pan
		_drag_coords[0] = Fl::event_x() - x(); _drag_coords[1] = Fl::event_y() - y();
		_state = _prev_state;
		pan(_drag_coords[0] - _click_coords[0], _drag_coords[1] - _click_coords[1]);
		return 1;
	case FL_MOUSEWHEEL:
		// zoom
		if (Fl::event_dy() > 0) { // scrolled down
			zoom_out(Fl::event_x(), Fl::event_y());
		}
		else if (Fl::event_dy() < 0) { // scrolled up
			zoom_in(Fl::event_x(), Fl::event_y());
		}
		return 1;
	}
	return Fl_Gl_Window::handle(event);
}

int Workspace::handle_3d(int event) {
	switch (event) {
	case FL_PUSH:
		// start rotating (left) or panning (middle/right)
		_click_coords[0] = Fl::event_x() - x(); _click_coords[1] = Fl::event_y() - y();
		_dragging = true;
		_left_mouse = Fl::event_button() == FL_LEFT_MOUSE;
		_prev_state = _state;
		return 1;
	case FL_RELEASE:
		// stop rotating or panning
		_drag_coords[0] = Fl::event_x() - x(); _drag_coords[1] = Fl::event_y() - y();
		_dragging = false;
		redraw();
		return 1;
	case FL_DRAG:
		_drag_coords[0] = Fl::event_x() - x(); _drag_coords[1] = Fl::event_y() - y();
		if (_left_mouse) {
			rotate();
		}
		else {
			pan(_drag_coords[0] - _click_coords[0], _drag_coords[1] - _click_coords[1]);
		}
		return 1;
	case FL_MOUSEWHEEL:
		// zoom
		if (Fl::event_dy() > 0) { // scrolled down
			zoom_out(Fl::event_x(), Fl::event_y());
		}
		else if (Fl::event_dy() < 0) { // scrolled up
			zoom_in(Fl::event_x(), Fl::event_y());
		}
		return 1;
	}
	return Fl_Gl_Window::handle(event);
}
