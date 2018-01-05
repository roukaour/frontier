#pragma once

#pragma warning(push, 0)
#include <FL/gl.h>
#include <FL/glu.h>
#include <FL/Fl_Gl_Window.H>
#pragma warning(pop)

#include "heightmap.h"
#include "draw-state.h"
#include "modal-dialogs.h"

class Workspace : public Fl_Gl_Window {
private:
	static const double FOV_Y;
	static const double NEAR_PLANE, FAR_PLANE;
	static const double FOCAL_LENGTH;
	static const double PAN_SCALE, ZOOM_SCALE;
private:
	bool _initialized, _opened, _dragging, _left_mouse;
	Heightmap _heightmap;
	Draw_State _state, _prev_state;
	int _click_coords[2], _drag_coords[2];
public:
	Workspace(int x, int y, int w, int h);
	inline bool opened(void) const { return _opened; }
	inline const Heightmap &heightmap(void) const { return _heightmap; }
	inline const Draw_State &draw_state(void) const { return _state; }
	bool create(size_t w, size_t h);
	bool open(const char *filename);
	bool save(const char *filename, Progress_Dialog *pd = NULL);
	void close(void);
	void rotate(void);
	void pan(int dx, int dy);
	void zoom_in(int cx, int cy);
	void zoom_out(int cx, int cy);
	void zoom_reset(int cx, int cy);
	void decimate(bool random, double thresh, Progress_Dialog *pd = NULL);
	bool expand(size_t power, Progress_Dialog *pd = NULL);
	void interpolate(bool mdbu, float I, bool md, float H, float rt, float rs, Progress_Dialog *pd = NULL);
	void erode(size_t nts, bool thermal, float Kt, float Ka, float Ki, bool hydraulic, float Kc, float Kd, float Ks,
		float Ke, float W0, float Wmin, Progress_Dialog *pd = NULL);
	bool calculate_normals(Progress_Dialog *pd = NULL);
	void render_3d(bool r);
	inline void color_scheme(Color_Scheme cs) { _state.color_scheme(cs); invalidate(); redraw(); }
	inline void scale(float s) { _state.scale(s); invalidate(); redraw(); }
protected:
	void draw(void);
	int handle(int event);
private:
	static void refresh_gl(void);
	void refresh_projection(void) const;
	void refresh_view(void);
	void draw_heightmap_2d(void);
	void draw_heightmap_3d(void);
	int handle_2d(int event);
	int handle_3d(int event);
};
