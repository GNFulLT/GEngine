#include "internal/engine/rendering/vulkan/gvulkan_shader_info.h"
#include "internal/engine/shader/spirv_parser.h"

GVulkanShaderInfo::GVulkanShaderInfo()
{
}

bool GVulkanShaderInfo::init(uint32_t* words, size_t size)
{
	auto wordCount = std::uint32_t(size / sizeof(std::uint32_t));

	SpvReflectShaderModule spvModule;
	auto res = spvReflectCreateShaderModule(size, words, &spvModule);
	if (res != SPV_REFLECT_RESULT_SUCCESS)
	{
		return false;
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

		VkWriteDescriptorSet wset = {};

		wset.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wset.pNext = nullptr;
		wset.descriptorType = binding.descriptorType;
		wset.descriptorCount = binding.descriptorCount;
		wset.dstBinding = (*iterSet)->binding;
		
		m_uniformBlockSizes.emplace(i,(*iterSet)->block.size);

		m_writeSets.push_back(wset);
		m_bindings.push_back(binding);
		m_bindingNames.push_back((*iterSet)->name);
	}

	spvReflectDestroyShaderModule(&spvModule);

	return true;
}

uint32_t GVulkanShaderInfo::get_binding_count() const
{
	return m_writeSets.size();
}

const VkWriteDescriptorSet* GVulkanShaderInfo::get_write_set_by_index(uint32_t index) const
{
	if (index >= m_writeSets.size())
		return nullptr;
	return &m_writeSets[index];
}

const VkDescriptorSetLayoutBinding* GVulkanShaderInfo::get_binding_by_index(uint32_t index) const
{
	if (index >= m_bindings.size())
		return nullptr;
	return &m_bindings[index];
}

const char* GVulkanShaderInfo::get_binding_name_by_index(uint32_t index) const
{
	if (index >= m_bindingNames.size())
		return nullptr;
	return m_bindingNames[index].c_str();
}

uint32_t GVulkanShaderInfo::get_uniform_buffer_size(uint32_t index) const
{
	auto iter = m_uniformBlockSizes.find(index);
	return iter == m_uniformBlockSizes.end() ? 0 : iter->second;
}

const std::vector<VkDescriptorSetLayoutBinding>* GVulkanShaderInfo::get_bindings() const
{
	return &m_bindings;
}
