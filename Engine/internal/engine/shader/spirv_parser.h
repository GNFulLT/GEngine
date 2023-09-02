#ifndef SPIRV_PARSER_H
#define SPIRV_PARSER_H

#include "volk.h"

#include <cstdint>
#include <vector>
#include <expected>

#include "engine/shader/ispirv_shader.h"
#include <spirv_reflect.h>
#include "internal/engine/shader/spirv_shader_utils.h"

enum SPIRV_READ_BINARY_RESULT
{
	SPIRV_READ_BINARY_RESULT_OK,
	SPIRV_READ_BINARY_RESULT_UNKNOWN
};

 SPIRV_READ_BINARY_RESULT get_bindings_from_binary(const uint32_t* words,std::size_t size, std::vector<std::pair<VkDescriptorSetLayoutBinding,VkWriteDescriptorSet>>& vec);

SPIRV_SHADER_STAGE reflect_shader_stage_to_spirv_stage(SpvReflectShaderStageFlagBits stage);

VkDescriptorType spv_descriptor_to_vk_descriptor(SpvReflectDescriptorType type);

#endif // SPIRV_PARSER_H