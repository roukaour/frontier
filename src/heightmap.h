#pragma once

#include <cstdlib>
#include <unordered_set>
#include <unordered_map>
#include <queue>

#include "draw-state.h"
#include "modal-dialogs.h"

// std::pair lacks a std::hash definition, so it cannot be used as a std::unordered_set key
// <http://stackoverflow.com/questions/15160889/how-to-make-unordered-set-of-pairs-of-integers-in-c>
namespace std {
	template <> struct hash<std::pair<size_t, size_t>> {
		inline size_t operator()(const std::pair<size_t, size_t> &v) const {
			std::hash<size_t> h;
			return h(v.first) ^ h(v.second);
		}
	};
}

typedef std::pair<size_t, size_t> Point;
typedef std::unordered_set<Point> Points;
typedef std::unordered_map<Point, Points> Point_to_Points;

struct Vector3 {
	union {
		struct {
			float x, y, z;
		};
		float xyz[3];
	};
};

struct Column {
	float elevation, hardness, solubility;
	Vector3 normal;
};

void column_color(Column &c, Color_Scheme cs, float *cv);

class Heightmap {
public:
	static const float UNKNOWN_ELEVATION;
	static const float DEFAULT_HARDNESS, DERIVED_HARDNESS_VARIANCE;
	static const float DEFAULT_SOLUBILITY, DERIVED_SOLUBILITY_VARIANCE;
private:
	Column *_heightmap;
	size_t _width, _height, _known_elevations;
public:
	Heightmap();
	inline Column &column(size_t i) const { return _heightmap[i]; }
	inline Column &column(size_t x, size_t y) const { return _heightmap[y * _width + x]; }
	inline float elevation(size_t i) const { return _heightmap[i].elevation; }
	inline float elevation(size_t x, size_t y) const { return _heightmap[y * _width + x].elevation; }
	inline float hardness(size_t i) const { return _heightmap[i].hardness; }
	inline float hardness(size_t x, size_t y) const { return _heightmap[y * _width + x].hardness; }
	inline float solubility(size_t i) const { return _heightmap[i].solubility; }
	inline float solubility(size_t x, size_t y) const { return _heightmap[y * _width + x].solubility; }
	inline void elevation(size_t x, size_t y, float e) { _heightmap[y * _width + x].elevation = e; }
	inline void hardness(size_t x, size_t y, float v) { _heightmap[y * _width + x].hardness = v; }
	inline void solubility(size_t x, size_t y, float s) { _heightmap[y * _width + x].solubility = s; }
	inline size_t width(void) const { return _width; }
	inline size_t height(void) const { return _height; }
	inline size_t known_elevations(void) const { return _known_elevations; }
	void clear(void);
	bool create(size_t w, size_t h);
	bool open(const char *filename);
	bool save(const char *filename, Color_Scheme cs, Progress_Dialog *pd = NULL) const;
	bool decimate(bool random, double thresh, Progress_Dialog *pd = NULL);
	bool decimate_random(double frac, Progress_Dialog *pd = NULL);
	bool decimate_edges(double thresh, Progress_Dialog *pd = NULL);
	bool expand(size_t power, Progress_Dialog *pd = NULL);
	bool interpolate(bool mdbu, float I, bool md, float H, float rt, float rs, Progress_Dialog *pd = NULL);
	bool erode(size_t nts, bool thermal, float Kt, float Ka, float Ki, bool hydraulic, float Kc, float Kd, float Ks,
		float Ke, float W0, float Wmin, Progress_Dialog *pd = NULL);
	bool calculate_normals(Progress_Dialog *pd = NULL);
private:
	bool open_png(const char *filename);
	bool save_png(const char *filename, Color_Scheme cs, Progress_Dialog *pd = NULL) const;
	bool md_bottom_up_diamond_square(float I, Progress_Dialog *pd = NULL);
	bool midpoint_displacement_diamond_square(float H, float rt, float rs, Progress_Dialog *pd = NULL);
	Points ascendants(size_t mx, size_t my) const;
	void sample_square(float px, float py, float hdx, float hdy, float rt, float rs);
	void sample_diamond(float px, float py, float hdx, float hdy, float rt, float rs);
};
