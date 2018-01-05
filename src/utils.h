#pragma once

enum Filetype { UNKNOWN, PNG };

Filetype file_type_by_extension(const char *filename);
