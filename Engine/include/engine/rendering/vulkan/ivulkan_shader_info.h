#ifndef IVULKAN_SHADER_INFO_H
#define IVULKAN_SHADER_INFO_H

#include "engine/GEngine_EXPORT.h"
#include <cstdint>
#include <vector>
struct VkWriteDescriptorSet;
struct VkDescriptorSetLayoutBinding;

class ENGINE_API IGVulkanShaderInfo {
public:
	virtual ~IGVulkanShaderInfo() = default;

	virtual uint32_t get_binding_count() const = 0;

	virtual const VkWriteDescriptorSet* get_write_set_by_index(uint32_t index) const = 0;

	virtual const VkDescriptorSetLayoutBinding* get_binding_by_index(uint32_t index) const = 0;
	
	virtual const char* get_binding_name_by_index(uint32_t index) const = 0;
	
	virtual const std::vector<VkDescriptorSetLayoutBinding>* get_bindings() const = 0;

	virtual uint32_t get_uniform_buffer_size(uint32_t index) const = 0;

private:

};

#endif //IVULKAN_SHADER_INFO_H