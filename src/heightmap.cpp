#include <cstdlib>
#include <string>
#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <png.h>
#include <ZLIB.H>

#pragma warning(push, 0)
#include <FL/Fl.H>
#include <FL/Fl_PNG_Image.H>
#pragma warning(pop)

#include "draw-state.h"
#include "algebra.h"
#include "heightmap.h"

#define HYDRAULIC_CORRECTION 50

const float Heightmap::UNKNOWN_ELEVATION = -1.0f;

const float Heightmap::DEFAULT_HARDNESS = 0.5f;
const float Heightmap::DERIVED_HARDNESS_VARIANCE = 0.08f;

const float Heightmap::DEFAULT_SOLUBILITY = 0.04f;
const float Heightmap::DERIVED_SOLUBILITY_VARIANCE = 0.08f;

void column_color(Column &c, Color_Scheme cs, float *cv) {
	if (c.elevation == Heightmap::UNKNOWN_ELEVATION) {
		cv[0] = cv[1] = cv[2] = 0.0f;
	}
	else if (cs == GRAYSCALE) {
		cv[0] = cv[1] = cv[2] = c.elevation;
	}
	else if (cs == ARTIFICIAL_EARTH) {
		cv[0] = 0.1875f + c.elevation * 0.8125f;
		cv[1] = 0.1875f + c.elevation * 0.8125f * 0.75f;
		cv[2] = 0.1875f + c.elevation * 0.8125f * 0.25f;
	}
	else {
		cv[0] = cs == ELEVATION_RED || cs == COMBINATION_WHITE ? c.elevation : 0.0f;
		cv[1] = cs == HARDNESS_GREEN || cs == COMBINATION_WHITE ? c.hardness : 0.0f;
		cv[2] = cs == SOLUBILITY_BLUE || cs == COMBINATION_WHITE ? c.solubility : 0.0f;
	}
}

Heightmap::Heightmap() : _heightmap(NULL), _width(0), _height(0), _known_elevations(0) {}

static float random01() {
	// Random float in [0, 1]
	return (float)rand() / (float)(RAND_MAX - 1);
}

static float random11() {
	// Random float in [-1, 1]
	return (float)rand() * 2.0f / (float)(RAND_MAX - 1) - 1.0f;
}

static float clamp01(float v) {
	// Clamp v to within [0, 1]
	return v < 0.0f ? 0.0f : v > 1.0f ? 1.0f : v;
}

void Heightmap::clear() {
	if (_heightmap) { delete [] _heightmap; }
	_heightmap = NULL;
	_width = _height = _known_elevations = 0;
}

bool Heightmap::create(size_t w, size_t h) {
	clear();
	size_t np = w * h;
	if (!np) { return false; }
	_heightmap = new(std::nothrow) Column[np]();
	if (!_heightmap) { return false; }
	_width = w; _height = h;
	for (size_t i = 0; i < np; i++) {
		_heightmap[i].elevation = UNKNOWN_ELEVATION;
		_heightmap[i].hardness = DEFAULT_HARDNESS;
		_heightmap[i].solubility = DEFAULT_SOLUBILITY;
	}
	_known_elevations = 0;
	return true;
}

static std::string file_extension(const char *filename) {
	std::string ext = filename;
	size_t last_dot = ext.find_last_of('.');
	if (last_dot == std::string::npos) { return ""; }
	ext = ext.substr(last_dot + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
	return ext;
}

bool Heightmap::open(const char *filename) {
	clear();
	std::string ext = file_extension(filename);
	if (ext == "png") { return open_png(filename); }
	return false;
}

static float derive_hardness(float h) {
	// Elevation plus noise, scaled lower, yields hardness
	return h == Heightmap::UNKNOWN_ELEVATION ? Heightmap::DEFAULT_HARDNESS :
		clamp01(h + random11() * Heightmap::DERIVED_HARDNESS_VARIANCE);
}

static float derive_solubility(float h) {
	// Elevation plus noise, scaled lower, yields solubility
	return h == Heightmap::UNKNOWN_ELEVATION ? Heightmap::DEFAULT_SOLUBILITY :
		clamp01(h + random11() * Heightmap::DERIVED_SOLUBILITY_VARIANCE);
}

bool Heightmap::open_png(const char *filename) {
	// PNG channels: red = elevation, green = hardness, blue = solubility; alpha of 0 = unknown elevation 
	Fl_PNG_Image image(filename);
	size_t np = image.w() * image.h();
	if (!np) { return false; }
	_heightmap = new(std::nothrow) Column[np]();
	if (!_heightmap) { return false; }
	_width = image.w(); _height = image.h();
	const unsigned char *pixels = (const unsigned char *)image.data()[0];
	int depth = image.d();
	switch (depth) {
	case 1: // grayscale
		for (size_t i = 0; i < np; i++) {
			const unsigned char *p = pixels + i * depth;
			_heightmap[i].elevation = (float)p[0] / 255.0f;
			_heightmap[i].hardness = derive_hardness(_heightmap[i].elevation);
			_heightmap[i].solubility = derive_solubility(_heightmap[i].elevation);
		}
		_known_elevations = np;
		break;
	case 2: // grayscale with alpha
		for (size_t i = 0; i < np; i++) {
			const unsigned char *p = pixels + i * depth;
			_heightmap[i].elevation = p[1] ? (float)p[0] / 255.0f : UNKNOWN_ELEVATION;
			if (_heightmap[i].elevation != UNKNOWN_ELEVATION) { _known_elevations++; }
			_heightmap[i].hardness = derive_hardness(_heightmap[i].elevation);
			_heightmap[i].solubility = derive_solubility(_heightmap[i].elevation);
		}
		break;
	case 3: // RGB
		for (size_t i = 0; i < np; i++) {
			const unsigned char *p = pixels + i * depth;
			_heightmap[i].elevation = (float)p[0] / 255.0f;
			_heightmap[i].hardness = (float)p[1] / 255.0f;
			_heightmap[i].solubility = (float)p[2] / 255.0f;
		}
		_known_elevations = np;
		break;
	case 4: // RGBA
		for (size_t i = 0; i < np; i++) {
			const unsigned char *p = pixels + i * depth;
			_heightmap[i].elevation = p[3] ? (float)p[0] / 255.0f : UNKNOWN_ELEVATION;
			_heightmap[i].hardness = p[3] ? (float)p[1] / 255.0f : DEFAULT_HARDNESS;
			_heightmap[i].solubility = p[3] ? (float)p[2] / 255.0f : DEFAULT_SOLUBILITY;
			if (_heightmap[i].elevation != UNKNOWN_ELEVATION) { _known_elevations++; }
		}
		break;
	default:
		return false;
	}
	return true;
}

bool Heightmap::save(const char *filename, Color_Scheme cs, Progress_Dialog *pd) const {
	return save_png(filename, cs, pd);
}

bool Heightmap::save_png(const char *filename, Color_Scheme cs, Progress_Dialog *pd) const {
	size_t denom = 1;
	if (pd) {
		pd->canceled(false);
	}
	FILE *file = fopen(filename, "wb");
	if (!file) { return false; }
	// Create the necessary PNG structures
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) {
		fclose(file);
		return false;
	}
	png_infop info = png_create_info_struct(png);
	if (!info) {
		png_destroy_write_struct(&png, (png_infopp)NULL);
		fclose(file);
		return false;
	}
	size_t np = _width * _height;
	if (pd) {
		denom = np / PROGRESS_STEPS;
		if (!denom) { denom = 1; }
		pd->message("Saving DTED...");
		pd->progress(0.0f);
		Fl::check();
		if (pd->canceled()) {
			png_destroy_write_struct(&png, &info);
			png_free_data(png, info, PNG_FREE_ALL, -1);
			fclose(file);
			return false;
		}
	}
	png_init_io(png, file);
	// Set compression options (commented out for being slow)
	/*
	png_set_compression_level(png, Z_BEST_COMPRESSION);
	png_set_compression_mem_level(png, Z_BEST_COMPRESSION);
	png_set_compression_strategy(png, Z_DEFAULT_STRATEGY);
	png_set_compression_window_bits(png, 15);
	png_set_compression_method(png, Z_DEFLATED);
	png_set_compression_buffer_size(png, 8192);
	*/
	// Write the PNG IHDR chunk
	png_set_IHDR(png, info, (png_uint_32)_width, (png_uint_32)_height, 8, PNG_COLOR_TYPE_RGB_ALPHA,
		PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	// Write the other PNG header chunks
	png_write_info(png, info);
	// Write the RGB pixels in row-major order from top to bottom
	png_bytep png_row = new(std::nothrow) png_byte[4 * _width];
	if (!png_row) {
		png_destroy_write_struct(&png, (png_infopp)NULL);
		fclose(file);
		return false;
	}
	for (size_t y = 0, i = 0; y < _height; y++) {
		for (size_t x = 0; x < _width; x++, i++) {
			Column &c = column(x, y);
			float h = c.elevation;
			float cv[3];
			column_color(c, cs, cv);
			size_t j = 4 * x;
			png_row[j] =   (png_byte)(cv[0] * 255.0f);
			png_row[j+1] = (png_byte)(cv[1] * 255.0f);
			png_row[j+2] = (png_byte)(cv[2] * 255.0f);
			png_row[j+3] = h == UNKNOWN_ELEVATION ? 0 : 255;
			if (pd && !((i + 1) % denom)) {
				pd->progress((float)(i + 1) / np);
				Fl::check();
				if (pd->canceled()) {
					delete [] png_row;
					png_destroy_write_struct(&png, &info);
					png_free_data(png, info, PNG_FREE_ALL, -1);
					fclose(file);
					return false;
				}
			}
			i++;
		}
		png_write_row(png, png_row);
	}
	// Write the end of the PNG
	png_write_end(png, NULL);
	delete [] png_row;
	png_destroy_write_struct(&png, &info);
	png_free_data(png, info, PNG_FREE_ALL, -1);
	fclose(file);
	if (pd) {
		pd->progress(1.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	return true;
}

bool Heightmap::decimate(bool random, double thresh, Progress_Dialog *pd) {
	return random ? decimate_random(thresh, pd) : decimate_edges(thresh, pd);
}

bool Heightmap::decimate_random(double frac, Progress_Dialog *pd) {
	size_t denom = 1;
	if (pd) {
		pd->canceled(false);
	}
	size_t np = _width * _height;
	if (pd) {
		denom = np / PROGRESS_STEPS;
		if (!denom) { denom = 1; }
		pd->message("Decimating...");
		pd->progress(0.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	// For each column, pick whether to remove it
	for (size_t i = 0; i < np; i++) {
		double r = random01();
		if (r < frac && _heightmap[i].elevation != UNKNOWN_ELEVATION) {
			_heightmap[i].elevation = UNKNOWN_ELEVATION;
			_known_elevations--;
		}
		if (pd && !((i + 1) % denom)) {
			pd->progress((float)(i + 1) / np);
			Fl::check();
			if (pd->canceled()) { return false; }
		}
	}
	if (pd) {
		pd->progress(1.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	return true;
}

bool Heightmap::decimate_edges(double thresh, Progress_Dialog *pd) {
	size_t denom = 1;
	if (pd) {
		pd->canceled(false);
	}
	size_t np = _width * _height;
	if (pd) {
		denom = np / PROGRESS_STEPS;
		if (!denom) { denom = 1; }
		pd->message("Decimating...");
		pd->progress(0.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	float *edgeness_map = new(std::nothrow) float[np]();
	if (!edgeness_map) { return false; }
	// Kernels for Sobel operator
	double gx[3][3] = {{1.0, 0.0, -1.0}, {2.0, 0.0, -2.0}, {1.0, 0.0, -1.0}};
	double gy[3][3] = {{1.0, 2.0, 1.0}, {0.0, 0.0, 0.0}, {-1.0, -2.0, -1.0}};
	float max_edgeness = 0.0;
	// For each column, compute its edgeness
	for (size_t y = 0, i = 0; y < _height; y++) {
		for (size_t x = 0; x < _width; x++, i++) {
			size_t i = y * _width + x;
			double edgeness = 0.0;
			if (y > 0 && y < _height - 1 && x > 0 && x < _width - 1 && elevation(x, y) != UNKNOWN_ELEVATION) {
				// Convolve the column's 3x3 neighborhood with the Sobel operator
				double edgeness_x = 0.0, edgeness_y = 0.0;
				for (int dy = -1; dy <= 1; dy++) {
					for (int dx = -1; dx <= 1; dx++) {
						edgeness_x += elevation(x + dx, y + dy) * gx[dy + 1][dx + 1];
						edgeness_y += elevation(x + dx, y + dy) * gy[dy + 1][dx + 1];
					}
				}
				edgeness = sqrt(edgeness_x * edgeness_x + edgeness_y * edgeness_y);
			}
			edgeness_map[i] = clamp01((float)edgeness);
			if (edgeness_map[i] > max_edgeness) { max_edgeness = edgeness_map[i]; }
			if (pd && !((i + 1) % denom)) {
				pd->progress((float)(i + 1) / (np * 2));
				Fl::check();
				if (pd->canceled()) {
					delete [] edgeness_map;
					return false;
				}
			}
		}
	}
	thresh = thresh * max_edgeness;
	// For each column, remove the columns that are not edgy enough
	for (size_t i = 0; i < np; i++) {
		if (edgeness_map[i] < thresh && _heightmap[i].elevation != UNKNOWN_ELEVATION) {
			_heightmap[i].elevation = UNKNOWN_ELEVATION;
			_known_elevations--;
		}
		if (pd && !((i + 1) % denom)) {
			pd->progress(0.5f + (float)(i + 1) / (np * 2));
			Fl::check();
			if (pd->canceled()) {
				delete [] edgeness_map;
				return false;
			}
		}
	}
	delete [] edgeness_map;
	if (pd) {
		pd->progress(1.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	return true;
}

bool Heightmap::expand(size_t power, Progress_Dialog *pd) {
	size_t denom = 1;
	if (pd) {
		pd->canceled(false);
	}
	size_t factor = (size_t)pow(2, power);
	size_t np = _width * _height;
	size_t new_width = (_width - 1) * factor + 1, new_height = (_height - 1) * factor + 1;
	size_t new_np = new_width * new_height;
	if (pd) {
		denom = new_np / PROGRESS_STEPS;
		if (!denom) { denom = 1; }
		pd->message("Expanding...");
		pd->progress(0.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	Column *new_heightmap = new(std::nothrow) Column[new_np]();
	if (!new_heightmap) { return false; }
	// Initialize expanded heightmap
	for (size_t i = 0; i < new_np; i++) {
		new_heightmap[i].elevation = UNKNOWN_ELEVATION;
		new_heightmap[i].hardness = DEFAULT_HARDNESS;
		new_heightmap[i].solubility = DEFAULT_SOLUBILITY;
		if (pd && !((i + 1) % denom)) {
			pd->progress((float)(i + 1) / new_np / 2.0f);
			Fl::check();
			if (pd->canceled()) {
				delete [] new_heightmap;
				return false;
			}
		}
	}
	if (pd) {
		denom = np / PROGRESS_STEPS;
		if (!denom) { denom = 1; }
	}
	// For each column, copy it to the expanded heightmap
	for (size_t y = 0, i = 0; y < _height; y++) {
		for (size_t x = 0; x < _width; x++, i++) {
			Column &c = column(x, y);
			size_t new_i = y * new_width * factor + x * factor;
			new_heightmap[new_i].elevation = c.elevation;
			new_heightmap[new_i].hardness = c.hardness;
			new_heightmap[new_i].solubility = c.solubility;
			if (pd && !((i + 1) % denom)) {
				pd->progress(0.5f + (float)(i + 1) / np / 2.0f);
				Fl::check();
				if (pd->canceled()) {
					delete [] new_heightmap;
					return false;
				}
			}
		}
	}
	_width = new_width; _height = new_height;
	delete [] _heightmap; _heightmap = new_heightmap;
	if (pd) {
		pd->progress(1.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	return true;
}

bool Heightmap::interpolate(bool mdbu, float I, bool md, float H, float rt, float rs, Progress_Dialog *pd) {
	// Morphologically Constrained Midpoint Displacement (MCMD) algorithm from
	// "Terrain Modeling: A Constrained Fractal Model" (Belhadj, 2007)
	if (_known_elevations == _width * _height) { return true; }
	if (pd) {
		pd->canceled(false);
	}
	if (mdbu && !md_bottom_up_diamond_square(I, pd)) { return false; }
	if (md && !midpoint_displacement_diamond_square(H, rt, rs, pd)) { return false; }
	if (pd) {
		pd->progress(1.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	return true;
}

static float euclidean_distance(Point a, Point b) {
	size_t dx = a.first > b.first ? a.first - b.first : b.first - a.first;
	size_t dy = a.second > b.second ? a.second - b.second : b.second - a.second;
	return sqrt((float)(dx * dx + dy * dy));
}

bool Heightmap::md_bottom_up_diamond_square(float I, Progress_Dialog *pd) {
	// Midpoint Displacement Bottom-Up (MDBU) step of MCMD algorithm, using diamond-square MD
	size_t np = _width * _height;
	float sigma = I < 0.0f ? -1.0f : 1.0f;
	I = fabs(I);
	size_t denom = 1;
	if (pd) {
		denom = (np * 2) / (PROGRESS_STEPS * 1);
		if (!denom) { denom = 1; }
		pd->message("Interpolating bottom-up...");
		pd->progress(0.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	std::queue<Point> FQ;
	size_t i = 0;
	for (size_t y = 0; y < _height; y++) {
		for (size_t x = 0; x < _width; x++) {
			float h = elevation(x, y);
			if (h != UNKNOWN_ELEVATION) {
				Point C(x, y);
				FQ.push(C);
				if (pd && !((i + 1) % denom)) {
					pd->progress((float)(i + 1) / (np * 2));
					Fl::check();
					if (pd->canceled()) { return false; }
				}
				i++;
			}
		}
	}
	float max_d = sqrt((float)np);
	while (!FQ.empty()) {
		Point_to_Points T;
		while (!FQ.empty()) {
			Point E = FQ.front();
			size_t Ex = E.first, Ey = E.second;
			Points As = ascendants(Ex, Ey);
			for (Points::const_iterator As_it = As.begin(); As_it != As.end(); ++As_it) {
				Point A = *As_it;
				size_t Ax = A.first, Ay = A.second;
				float Ah = elevation(Ax, Ay);
				if (Ah == UNKNOWN_ELEVATION) {
					T[A].insert(E);
				}
			}
			FQ.pop();
			if (pd && !((i + 1) % denom)) {
				pd->progress((float)(i + 1) / (np * 2));
				Fl::check();
				if (pd->canceled()) { return false; }
			}
			i++;
		}
		for (Point_to_Points::iterator T_it = T.begin(); T_it != T.end(); ++T_it) {
			Point A = (*T_it).first;
			Points Cs = (*T_it).second;
			float ce = 0.0f, ch = 0.0f, cs = 0.0f;
			size_t n = 0;
			for (Points::const_iterator Cs_it = Cs.begin(); Cs_it != Cs.end(); ++Cs_it) {
				Point C = *Cs_it;
				float weight = 1.0f - sigma * (1.0f - pow(1.0f - euclidean_distance(A, C) / max_d, I));
				Column c = column(C.first, C.second);
				ce += c.elevation * weight;
				ch += c.hardness * weight;
				cs += c.solubility * weight;
				n++;
			}
			Column &c = column(A.first, A.second);
			c.elevation = ce / n;
			c.hardness = ch / n;
			c.solubility = cs / n;
			_known_elevations++;
			FQ.push(A);
			if (pd && !((i + 1) % denom)) {
				pd->progress((float)(i + 1) / (np * 2));
				Fl::check();
				if (pd->canceled()) { return false; }
			}
			i++;
		}
	}
	return true;
}

Points Heightmap::ascendants(size_t mx, size_t my) const {
	Points as;
	if ((mx == 0 || mx == _width - 1) && (my == 0 || my == _height - 1)) {
		return as;
	}
	for (float dx = (float)(_width - 1), dy = (float)(_height - 1); /*true*/; dx /= 2.0f, dy /= 2.0f) {
		float hdx = dx / 2.0f, hdy = dy / 2.0f;
		// square
		for (float py = hdy; py < _height; py += dy) {
			if ((size_t)floor(py) != my) { continue; }
			for (float px = hdx; px < _width; px += dx) {
				if ((size_t)floor(px) != mx) { continue; }
				size_t x0 = (size_t)floor(px - hdx), x1 = (size_t)floor(px + hdx);
				size_t y0 = (size_t)floor(py - hdy), y1 = (size_t)floor(py + hdy);
				if (px >= hdx && py >= hdy) { as.insert(std::make_pair(x0, y0)); }
				if (x1 < _width && py >= hdy) { as.insert(std::make_pair(x1, y0)); }
				if (px >= hdx && y1 < _height) { as.insert(std::make_pair(x0, y1)); }
				if (x1 < _width && y1 < _height) { as.insert(std::make_pair(x1, y1)); }
				return as;
			}
		}
		// diamond
		for (float py = 0.0f; py < _height; py += dy) {
			float bdx = hdx, bdy = 0.0f;
			if ((size_t)floor(py + bdy) != my) { bdx = 0.0f; bdy = hdy; }
			if ((size_t)floor(py + bdy) != my) { continue; }
			for (float px = 0.0f; px < _width; px += dx) {
				if ((size_t)floor(px + bdx) != mx) { continue; }
				size_t x0 = (size_t)floor(px + bdx - hdx), x1 = (size_t)floor(px + bdx + hdx);
				size_t y0 = (size_t)floor(py + bdy - hdy), y1 = (size_t)floor(py + bdy + hdy);
				if (px + bdx >= hdx) { as.insert(std::make_pair(x0, my)); }
				if (x1 < _width) { as.insert(std::make_pair(x1, my)); }
				if (py + bdy >= hdy) { as.insert(std::make_pair(mx, y0)); }
				if (y1 < _height) { as.insert(std::make_pair(mx, y1)); }
				return as;
			}
		}
	}
}

bool Heightmap::midpoint_displacement_diamond_square(float H, float rt, float rs, Progress_Dialog *pd) {
	// Midpoint Displacement (MD) step of MCMD algorithm, using diamond-square MD
	size_t denom = 1;
	size_t np = _width * _height;
	if (pd) {
		denom = np / PROGRESS_STEPS;
		if (!denom) { denom = 1; }
		pd->message("Interpolating top-down...");
		pd->progress(0.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	// Ensure that corners exist
	for (size_t cy = 0; cy < _height; cy += _height - 1) {
		for (size_t cx = 0; cx < _width; cx += _width - 1) {
			if (elevation(cx, cy) == UNKNOWN_ELEVATION) { elevation(cx, cy, (float)random01()); }
		}
	}
	// Diamond-square algorithm
	float dx = (float)(_width - 1), dy = (float)(_height - 1);
	size_t i = 0;
	while (dx > 0.5f && dy > 0.5f) {
		float hdx = dx / 2.0f, hdy = dy / 2.0f;
		// Squares
		for (float py = hdy; py < _height; py += dy) {
			for (float px = hdx; px < _width; px += dx) {
				sample_square(px, py, hdx, hdy, rt, rs);
				if (pd && !((i + 1) % denom)) {
					pd->progress((float)(i + 1) / np);
					Fl::check();
					if (pd->canceled()) { return false; }
				}
				i++;
			}
		}
		// Diamonds
		for (float py = 0.0f; py < _height; py += dy) {
			for (float px = 0.0f; px < _width; px += dx) {
				sample_diamond(px + hdx, py, hdx, hdy, rt, rs);
				if (pd && !((i + 1) % denom)) {
					pd->progress((float)(i + 1) / np);
					Fl::check();
					if (pd->canceled()) { return false; }
				}
				i++;
				sample_diamond(px, py + hdy, hdx, hdy, rt, rs);
				if (pd && !((i + 1) % denom)) {
					pd->progress((float)(i + 1) / np);
					Fl::check();
					if (pd->canceled()) { return false; }
				}
				i++;
			}
		}
		dx = hdx; dy = hdy;
		rs *= pow(2.0f, -H);
	}
	if (pd) {
		pd->progress(1.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	return true;
}

void Heightmap::sample_square(float px, float py, float hdx, float hdy, float rt, float rs) {
	size_t mx = (size_t)floor(px), my = (size_t)floor(py);
	if (px < 0.0f || mx >= _width || py < 0.0f || my >= _height) { return; }
	if (elevation(mx, my) != UNKNOWN_ELEVATION) { return; }
	size_t x0 = (size_t)floor(px - hdx), x1 = (size_t)floor(px + hdx);
	size_t y0 = (size_t)floor(py - hdy), y1 = (size_t)floor(py + hdy);
	size_t denom = 0;
	float mean = 0.0f;
	if (px >= hdx && py >= hdy) { mean += elevation(x0, y0); denom++; }
	if (x1 < _width && py >= hdy) { mean += elevation(x1, y0); denom++; }
	if (px >= hdx && y1 < _height) { mean += elevation(x0, y1); denom++; }
	if (x1 < _width && y1 < _height) { mean += elevation(x1, y1); denom++; }
	mean /= denom;
	float noise = (random11() + rt) * rs;
	float value = clamp01(mean + noise);
	Column &c = column(mx, my);
	c.elevation = value;
	c.hardness = derive_hardness(c.elevation);
	c.solubility = derive_solubility(c.elevation);
	_known_elevations++;
}

void Heightmap::sample_diamond(float px, float py, float hdx, float hdy, float rt, float rs) {
	size_t mx = (size_t)floor(px), my = (size_t)floor(py);
	if (px < 0.0f || mx >= _width || py < 0.0f || my >= _height) { return; }
	if (elevation(mx, my) != UNKNOWN_ELEVATION) { return; }
	size_t x0 = (size_t)floor(px - hdx), x1 = (size_t)floor(px + hdx);
	size_t y0 = (size_t)floor(py - hdy), y1 = (size_t)floor(py + hdy);
	size_t denom = 0;
	float mean = 0.0f;
	if (px >= hdx) { mean += elevation(x0, my); denom++; }
	if (x1 < _width) { mean += elevation(x1, my); denom++; }
	if (py >= hdy) { mean += elevation(mx, y0); denom++; }
	if (y1 < _height) { mean += elevation(mx, y1); denom++; }
	mean /= denom;
	float noise = (random11() + rt) * rs;
	float value = clamp01(mean + noise);
	Column &c = column(mx, my);
	c.elevation = value;
	c.hardness = derive_hardness(c.elevation);
	c.solubility = derive_solubility(c.elevation);
	_known_elevations++;
}

bool Heightmap::erode(size_t nts, bool thermal, float Kt, float Ka, float Ki, bool hydraulic, float Kc, float Kd,
	float Ks, float Ke, float W0, float Wmin, Progress_Dialog *pd) {
	// Thermal and hydraulic erosion algorithms from
	// "Fast Hydraulic and Thermal Erosion on the GPU" (Jako, 2011),
	// "Physically Based Hydraulic Erosion Simulation on Graphics Processing Unit" (Anh et al., 2007), and
	// "The Synthesis and Rendering of Eroded Fractal Terrains" (Musgrave, 1989)
	if (pd) {
		pd->canceled(false);
	}
	bool success = false;
	size_t denom = 1;
	size_t np = _width * _height;
	if (pd) {
		denom = np * nts / (PROGRESS_STEPS * 10);
		if (!denom) { denom = 1; }
		char *message = thermal ?
			(hydraulic ? "Applying hydraulic and thermal erosion..." : "Applying thermal erosion...") :
			(hydraulic ? "Applying hydraulic erosion..." : "No erosion");
		pd->message(message);
		pd->progress(0.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	float *elevation_deltas = new(std::nothrow) float[np]();
	float *water_deltas = new(std::nothrow) float[np]();
	float *sediment_deltas = new(std::nothrow) float[np]();
	float *water_map = new(std::nothrow) float[np]();
	float *sediment_map = new(std::nothrow) float[np]();
	float *min_talus_angles = new(std::nothrow) float[np]();
	if (!elevation_deltas || !water_deltas || !sediment_deltas || !water_map || !sediment_map || !min_talus_angles) {
		goto cleanup;
	}
	// Rainfall
	for (size_t i = 0; i < np; i++) {
		water_map[i] = Wmin + W0 * elevation(i);
		min_talus_angles[i] = atan(_heightmap[i].hardness * Ka + Ki);
	}
	// Iterate erosion over time
	size_t ti = 0;
	for (size_t t = 0; t < nts; t++) {
		// Reset material deltas
		for (size_t i = 0; i < np; i++) {
			elevation_deltas[i] = water_deltas[i] = sediment_deltas[i] = 0.0f;
		}
		// For each column, handle erosion processes
		for (size_t y = 0; y < _height; y++) {
			for (size_t x = 0; x < _width; x++) {
				size_t i = y * _width + x;
				float min_talus_angle = min_talus_angles[i];
				Column &c = column(i);
				float local_elevation_diffs[3][3] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
				float total_elevation_diff = 0.0f, total_talus_diff = 0.0f, max_elevation_diff = 0.0f;
				// For each neighbor, find their elevation difference
				for (int dy = -1; dy <= 1; dy++) {
					if ((y == 0 && dy == -1) || (y == _height - 1 && dy == 1)) { continue; }
					size_t oy = y + dy;
					for (int dx = -1; dx <= 1; dx++) {
						if ((x == 0 && dx == -1) || (x == _width - 1 && dx == 1) || (dy == 0 && dx == 0)) { continue; }
						size_t ox = x + dx;
						float oh = elevation(ox, oy);
						float elevation_diff = c.elevation - oh;
						if (elevation_diff <= 0.0f) { continue; }
						local_elevation_diffs[dy + 1][dx + 1] = elevation_diff;
						total_elevation_diff += elevation_diff;
						float distance = (dx != 0 && dy != 0 ? (float)SQRT_2 : 1.0f) / 255.0f; // corners are more distant
						float talus_angle = atan2(elevation_diff, distance);
						if (talus_angle >= min_talus_angle) {
							local_elevation_diffs[dy + 1][dx + 1] *= -1; // negative differences indicate use for thermal erosion
							if (elevation_diff > max_elevation_diff) { max_elevation_diff = elevation_diff; }
							total_talus_diff += elevation_diff;
						}
					}
				}
				// Thermal erosion
				if (thermal && total_talus_diff > 0.0) {
					float talus = Kt * (1.0f - c.hardness) * max_elevation_diff / 2.0f;
					// Remove talus from column
					elevation_deltas[i] -= talus;
					// Move talus proportionally to each neighbor
					for (int dy = -1; dy <= 1; dy++) {
						if ((y == 0 && dy == -1) || (y == _height - 1 && dy == 1)) { continue; }
						size_t oy = y + dy;
						for (int dx = -1; dx <= 1; dx++) {
							if ((x == 0 && dx == -1) || (x == _width - 1 && dx == 1) || (dy == 0 && dx == 0)) { continue; }
							size_t ox = x + dx;
							float elevation_diff = -local_elevation_diffs[dy + 1][dx + 1]; // re-negate difference
							if (elevation_diff == 0.0f) { continue; }
							float neighbor_proportion = elevation_diff / total_talus_diff;
							float talus_delta = talus * neighbor_proportion;
							elevation_deltas[oy * _width + ox] += talus_delta;
						}
					}
				}
				// Hydraulic erosion
				if (hydraulic) {
					if (total_elevation_diff <= 0.0 || water_map[i] == 0.0) {
						// Deposit sediment on column
						float sediment_delta = Kd * sediment_map[i];
						elevation_deltas[i] += sediment_delta;
						sediment_deltas[i] -= sediment_delta;
					}
					else {
						// Move materials to each neighbor
						for (int dy = -1; dy <= 1; dy++) {
							if ((y == 0 && dy == -1) || (y == _height - 1 && dy == 1)) { continue; }
							size_t oy = y + dy;
							for (int dx = -1; dx <= 1; dx++) {
								if ((x == 0 && dx == -1) || (x == _width - 1 && dx == 1) || (dy == 0 && dx == 0)) { continue; }
								size_t ox = x + dx;
								size_t j = oy * _width + ox;
								float elevation_diff = fabs(local_elevation_diffs[dy + 1][dx + 1]); // undo negation
								if (elevation_diff == 0.0f) { continue; }
								float water_diff = water_map[i] - water_map[j];
								float neighbor_proportion = elevation_diff / total_elevation_diff;
								// Flow water to neighbor
								float water_delta = neighbor_proportion * (elevation_diff + water_diff);
								if (water_delta > water_map[i]) { water_delta = water_map[i]; }
								water_deltas[i] -= water_delta;
								water_deltas[j] += water_delta;
								// Transport and/or deposit sediment
								float sediment_capacity = Kc * water_delta;
								if (sediment_map[i] >= sediment_capacity) {
									// Transport water's capacity of sediment to neighbor; deposit excess on column
									sediment_deltas[j] += sediment_capacity;
									float sediment_deposited = Kd * (neighbor_proportion * sediment_map[i] - sediment_capacity);
									elevation_deltas[i] += sediment_deposited * HYDRAULIC_CORRECTION;
									sediment_deltas[i] -= sediment_capacity + sediment_deposited;
								}
								else {
									// Transport sediment to neighbor; dissolve soil into water's excess capacity
									float sediment_transported = neighbor_proportion * sediment_map[i];
									float soil_dissolved = Ks * c.solubility * (sediment_capacity - sediment_transported);
									sediment_deltas[j] += sediment_transported + soil_dissolved;
									elevation_deltas[i] -= soil_dissolved * HYDRAULIC_CORRECTION;
									sediment_deltas[i] -= sediment_transported;
								}
							}
						}
					}
				}
				if (pd && !((ti + 1) % denom)) {
					pd->progress((float)(ti + 1) / (np * nts));
					Fl::check();
					if (pd->canceled()) { goto cleanup; }
				}
				ti++;
			}
		}
		// Distribute material deltas
		for (size_t i = 0; i < np; i++) {
			if (_heightmap[i].elevation == UNKNOWN_ELEVATION) { continue; }
			_heightmap[i].elevation += elevation_deltas[i];
			water_map[i] += water_deltas[i];
			sediment_map[i] += sediment_deltas[i];
		}
		if (hydraulic) {
			// Evaporation
			for (size_t i = 0; i < np; i++) {
				water_map[i] *= Ke;
			}
		}
	}
	if (pd) {
		pd->progress(1.0f);
		Fl::check();
		if (pd->canceled()) { goto cleanup; }
	}
	success = true;
cleanup:
	delete [] elevation_deltas;
	delete [] water_deltas;
	delete [] sediment_deltas;
	delete [] water_map;
	delete [] sediment_map;
	delete [] min_talus_angles;
	return success;
}

bool Heightmap::calculate_normals(Progress_Dialog *pd) {
	size_t denom = 1;
	if (pd) {
		pd->canceled(false);
	}
	size_t np = _width * _height;
	size_t nt = (_width - 1) * (_height - 1) * 2;
	if (pd) {
		denom = (nt + nt + np) / (PROGRESS_STEPS * 10);
		if (!denom) { denom = 1; }
		pd->message("Calculating face normals...");
		pd->progress(0.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	Vector3 *face_normals = new(std::nothrow) Vector3[nt]();
	if (!face_normals) { return false; }
	size_t i = 0;
	// For each triangle, calculate the normal of its face
	for (size_t y = 0; y < _height - 1; y++) {
		for (size_t x = 0; x < _width - 1; x++) {
			float ha = elevation(x, y);
			float hb = elevation(x + 1, y);
			float hc = elevation(x + 1, y + 1);
			float hd = elevation(x, y + 1);
			// triangle made of <x, y, ha>, <x, y+1, hd>, <x+1, y, hb>
			// V = <x, y+1, hd> - <x, y, ha> = <0, 1, hd-ha>
			// W = <x+1, y, hb> - <x, y, ha> = <1, 0, hb-ha>
			// N = V x W = <hb-ha, hd-ha, -1>
			face_normals[i].x = hb - ha;
			face_normals[i].y = hd - ha;
			face_normals[i].z = -1.0f;
			i++;
			if (pd && !((i + 1) % denom)) {
				pd->progress((float)(i + 1) / (nt + nt + np));
				Fl::check();
				if (pd->canceled()) {
					delete [] face_normals;
					return false;
				}
			}
			// triangle made of <x+1, y+1, hc>, <x+1, y, hb>, <x, y+1, hd>
			// V = <x+1, y, hb> - <x+1, y+1, hc> = <0, -1, hb-hc>
			// W = <x, y+1, hd> - <x+1, y+1, hc> = <-1, 0, hd-hc>
			// N = V x W = <hc-hd, hd-hc, -1>
			face_normals[i].x = hc - hd;
			face_normals[i].y = hd - hc;
			face_normals[i].z = -1.0f;
			i++;
			if (pd && !((i + 1) % denom)) {
				pd->progress((float)(i + 1) / (nt + nt + np));
				Fl::check();
				if (pd->canceled()) {
					delete [] face_normals;
					return false;
				}
			}
		}
	}
	if (pd) {
		pd->message("Adding face normals...");
		pd->progress((float)nt / (nt + nt + np));
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	i = 0;
	// For each column, add the normals of the triangles for which it is a vertex
	for (size_t y = 0; y < _height - 1; y++) {
		for (size_t x = 0; x < _width - 1; x++) {
			Column &ca = column(x, y);
			ca.normal.x += face_normals[i].x;
			ca.normal.y += face_normals[i].y;
			ca.normal.z += face_normals[i].z;
			Column cb = column(x + 1, y);
			cb.normal.x += face_normals[i].x;
			cb.normal.y += face_normals[i].y;
			cb.normal.z += face_normals[i].z;
			Column &cd = column(x, y + 1);
			cd.normal.x += face_normals[i].x;
			cd.normal.y += face_normals[i].y;
			cd.normal.z += face_normals[i].z;
			i++;
			if (pd && !((i + 1) % denom)) {
				pd->progress((float)(nt + i + 1) / (nt + nt + np));
				Fl::check();
				if (pd->canceled()) {
					delete [] face_normals;
					return false;
				}
			}
			Column cc = column(x + 1, y + 1);
			cc.normal.x += face_normals[i].x;
			cc.normal.y += face_normals[i].y;
			cc.normal.z += face_normals[i].z;
			cb.normal.x += face_normals[i].x;
			cb.normal.y += face_normals[i].y;
			cb.normal.z += face_normals[i].z;
			cd.normal.x += face_normals[i].x;
			cd.normal.y += face_normals[i].y;
			cd.normal.z += face_normals[i].z;
			i++;
			if (pd && !((i + 1) % denom)) {
				pd->progress((float)(nt + i + 1) / (nt + nt + np));
				Fl::check();
				if (pd->canceled()) {
					delete [] face_normals;
					return false;
				}
			}
		}
	}
	if (pd) {
		pd->message("Averaging face normals...");
		pd->progress((float)(nt + nt) / (nt + nt + np));
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	// For each column, normalize its normal
	for (i = 0; i < np; i++) {
		float triangles = -_heightmap[i].normal.z; // all face normals have z = -1, so summing n of them gives z = -n
		_heightmap[i].normal.x /= triangles;
		_heightmap[i].normal.y /= triangles;
		_heightmap[i].normal.z /= triangles;
		if (pd && !((i + 1) % denom)) {
			pd->progress((float)(nt + nt + i + 1) / (nt + nt + np));
			Fl::check();
			if (pd->canceled()) {
				delete [] face_normals;
				return false;
			}
		}
	}
	delete [] face_normals;
	if (pd) {
		pd->progress(1.0f);
		Fl::check();
		if (pd->canceled()) { return false; }
	}
	return true;
}
