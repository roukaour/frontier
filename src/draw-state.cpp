#include "algebra.h"
#include "draw-state.h"

const double Draw_State::MIN_ZOOM[2] = {0.25, 0.0001};
const double Draw_State::MAX_ZOOM[2] = {64.0, 4.0};

Draw_State::Draw_State() : _render_3d(false), _color_scheme(GRAYSCALE), _scale(100.0f) {
	reset();
}

void Draw_State::rotate(const double m[16]) {
	for (int i = 0; i < 16; i++) {
		_rotate_matrix[i] = m[i];
	}
}

void Draw_State::zoom(double z) {
	size_t zi = _render_3d ? 1 : 0;
	if (z < MIN_ZOOM[zi]) { z = MIN_ZOOM[zi]; }
	else if (z > MAX_ZOOM[zi]) { z = MAX_ZOOM[zi]; }
	_zoom_factor = z;
}

void Draw_State::zoom(double z, double cx, double cy) {
	double pz = zoom();
	zoom(z);
	if (!_render_3d) {
		pan_x(pan_x() + cx / zoom() - cx / pz);
		pan_y(pan_y() + cy / zoom() - cy / pz);
	}
}

void Draw_State::reset() {
	identity_matrix(_rotate_matrix);
	_pan_vector[0] = _pan_vector[1] = 0.0;
	_zoom_factor = 1.0;
}
