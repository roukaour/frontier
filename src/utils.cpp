#include <string>
#include <algorithm>

#include "utils.h"

Filetype file_type_by_extension(const char *filename) {
	std::string ext = filename;
	size_t last_dot = ext.find_last_of('.');
	if (last_dot == std::string::npos) { return UNKNOWN; }
	ext = ext.substr(last_dot + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), tolower);
	if (ext == "png") { return PNG; }
	return UNKNOWN;
}
