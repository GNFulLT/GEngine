#ifndef UTILS_H
#define UTILS_H

#include <string>
#include "editor/file_type.h"
#include <vector>

inline static const std::vector<std::string> all_glsl_files = { ".glsl",".glsl_frag",".glsl_vert",".glsl_geom",".glsl_comp",".glsl_tesc",".glsl_tese" };
inline static const std::vector<std::string> all_hlsl_files = { ".hlsl",".hlsl_frag",".hlsl_vert",".hlsl_geom",".hlsl_comp",".hlsl_tesc",".hlsl_tese" };



int endsWith(const char* s, const char* part);

FILE_TYPE get_file_type_from_name(const char* s);

bool is_hlsl_file(const char* str);
bool is_glsl_file(const char* str);

#endif // UTILS_H