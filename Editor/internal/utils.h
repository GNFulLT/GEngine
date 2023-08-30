#ifndef UTILS_H
#define UTILS_H

#include "editor/file_type.h"

int endsWith(const char* s, const char* part);

FILE_TYPE get_file_type_from_name(const char* s);

#endif // UTILS_H