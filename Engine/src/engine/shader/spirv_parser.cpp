#include "internal/engine/shader/spirv_parser.h"
SPIRV_READ_BINARY_RESULT get_bindings_from_binary(const uint32_t* words, std::size_t size, std::vector<VkDescriptorSetLayoutBinding>& vec)
{

	auto wordCount = std::uint32_t(size / sizeof(std::uint32_t));

	SpvReflectShaderModule spvModule;
	auto res = spvReflectCreateShaderModule(size, words, &spvModule);
	if (res != SPV_REFLECT_RESULT_SUCCESS)
	{
		return SPIRV_READ_BINARY_RESULT_UNKNOWN;
	}

	auto stg = reflect_shader_stage_to_spirv_stage(spvModule.shader_stage);
	auto stage = spirv_stage_to_vk_stage(stg);
	
	auto descriptorSet = spvModule.descriptor_sets->bindings;
	for (int i = 0; i < spvModule.descriptor_sets->binding_count; i++)
	{
		auto iterSet = descriptorSet + i;
		VkDescriptorSetLayoutBinding binding = {};
		binding.stageFlags = stage;
		binding.pImmutableSamplers = nullptr;
		binding.descriptorCount = (*iterSet)->count;
		binding.descriptorType = spv_descriptor_to_vk_descriptor((*iterSet)->descriptor_type);

		vec.push_back(binding);
	}
	return SPIRV_READ_BINARY_RESULT_OK;
}

SPIRV_SHADER_STAGE reflect_shader_stage_to_spirv_stage(SpvReflectShaderStageFlagBits stage)
{
	switch (stage)
	{
	case SPV_REFLECT_SHADER_STAGE_VERTEX_BIT:
		return SPIRV_SHADER_STAGE_VERTEX;
	case SPV_REFLECT_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
		return SPIRV_SHADER_STAGE_TESSCONTROL;
	case SPV_REFLECT_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
		return SPIRV_SHADER_STAGE_TESSEVALUATION;
	case SPV_REFLECT_SHADER_STAGE_GEOMETRY_BIT:
		return SPIRV_SHADER_STAGE_GEOMETRY;
	case SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT:
		return SPIRV_SHADER_STAGE_FRAGMENT;
	case SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT:
		return SPIRV_SHADER_STAGE_COMPUTE;
	case SPV_REFLECT_SHADER_STAGE_TASK_BIT_NV:
	case SPV_REFLECT_SHADER_STAGE_MESH_BIT_NV:
	case SPV_REFLECT_SHADER_STAGE_RAYGEN_BIT_KHR:
	case SPV_REFLECT_SHADER_STAGE_ANY_HIT_BIT_KHR:
	case SPV_REFLECT_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
	case SPV_REFLECT_SHADER_STAGE_MISS_BIT_KHR:
	case SPV_REFLECT_SHADER_STAGE_INTERSECTION_BIT_KHR:
	case SPV_REFLECT_SHADER_STAGE_CALLABLE_BIT_KHR:
	default:
		return SPIRV_SHADER_STAGE_UNKNOWN;
	}
}

VkDescriptorType spv_descriptor_to_vk_descriptor(SpvReflectDescriptorType type)
{
	switch (type)
	{
	case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
	case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
		return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
		return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
		return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
		return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
	default:
		return VK_DESCRIPTOR_TYPE_MAX_ENUM;
	}
}
