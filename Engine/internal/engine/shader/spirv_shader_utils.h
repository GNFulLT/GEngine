#ifndef SPIRV_SHADER_UTILS_H
#define SPIRV_SHADER_UTILS_H

#include <expected>
#include <string>
#include <glslang_c_interface.h>

#include "engine/shader/ispirv_shader.h"
#include "internal/engine/utils.h"
#include <utility>

enum READ_SHADER_FILE_ERROR
{
	READ_SHADER_FILE_ERROR_FILE_NOT_FOUND,
	READ_SHADER_FILE_ERROR_INCLUDE_NOT_FOUND
};



std::expected<std::string, READ_SHADER_FILE_ERROR> read_shader_file(const char* fileName);

std::pair<SPIRV_SHADER_STAGE, SPIRV_SOURCE_TYPE> shader_stage_from_file_name(const char* fileName);

glslang_stage_t spirv_shader_stage_to_glslang_stage(SPIRV_SHADER_STAGE stage);

glslang_source_t spirv_source_type_to_glslang_source(SPIRV_SOURCE_TYPE type);

SPIRV_SHADER_STAGE glslang_stage_to_spirv_shader_stage(glslang_stage_t stage);

SPIRV_SOURCE_TYPE glslang_source_to_spirv_source_type(glslang_source_t type);

glslang_target_client_version_t vulkan_version_to_glslang_version(uint32_t vers);

#endif // SPIRV_SHADER_UTILS_H