#ifndef GVULKAN_SHADER_INFO_H
#define GVULKAN_SHADER_INFO_H

#include "engine/rendering/vulkan/ivulkan_shader_info.h"
#include <string>
#include <vector>
#include <unordered_map>

class GVulkanShaderInfo : public IGVulkanShaderInfo
{
public:
	GVulkanShaderInfo();

	bool init(uint32_t* words,size_t size);


	// Inherited via IGVulkanShaderInfo
	virtual uint32_t get_binding_count() const override;
	virtual const VkWriteDescriptorSet* get_write_set_by_index(uint32_t index) const override;
	virtual const VkDescriptorSetLayoutBinding* get_binding_by_index(uint32_t index) const override;
	virtual const char* get_binding_name_by_index(uint32_t index) const override;

	virtual uint32_t get_uniform_buffer_size(uint32_t index) const override;
private:
	std::vector<VkWriteDescriptorSet> m_writeSets;
	std::vector< VkDescriptorSetLayoutBinding> m_bindings;
	std::vector<std::string> m_bindingNames;
	std::unordered_map<uint32_t, uint32_t> m_uniformBlockSizes;
	// Inherited via IGVulkanShaderInfo
	virtual const std::vector<VkDescriptorSetLayoutBinding>* get_bindings() const override;


};


#endif // GVULKAN_SHADER_INFO_H