#pragma once

enum Color_Scheme { GRAYSCALE, ELEVATION_RED, HARDNESS_GREEN, SOLUBILITY_BLUE, COMBINATION_WHITE, ARTIFICIAL_EARTH };

class Draw_State {
public:
	static const double MIN_ZOOM[2], MAX_ZOOM[2];
private:
	bool _render_3d;
	double _rotate_matrix[16];
	double _pan_vector[2];
	double _zoom_factor;
	Color_Scheme _color_scheme;
	float _scale;
public:
	Draw_State();
	inline const bool render_3d(void) const { return _render_3d; }
	inline void render_3d(bool r) { _render_3d = r; zoom(zoom()); }
	inline const double *rotate(void) const { return _rotate_matrix; }
	void rotate(const double m[16]);
	inline const double *pan(void) const { return _pan_vector; }
	inline void pan(const double v[2]) { _pan_vector[0] = v[0]; _pan_vector[1] = v[1]; }
	inline void pan(double x, double y) { _pan_vector[0] = x; _pan_vector[1] = y; }
	inline double pan_x(void) const { return _pan_vector[0]; }
	inline void pan_x(double x) { _pan_vector[0] = x; }
	inline double pan_y(void) const { return _pan_vector[1]; }
	inline void pan_y(double y) { _pan_vector[1] = y; }
	inline double zoom(void) const { return _zoom_factor; }
	void zoom(double z);
	void zoom(double z, double cx, double cy);
	inline Color_Scheme color_scheme(void) const { return _color_scheme; }
	inline void color_scheme(Color_Scheme cs) { _color_scheme = cs; }
	inline float scale(void) const { return _scale; }
	inline void scale(float s) { _scale = s; }
	void reset(void);
};
