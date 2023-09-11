#ifndef GVULKAN_NAMED_SET_LAYOUT_H
#define GVULKAN_NAMED_SET_LAYOUT_H

class IGVulkanLogicalDevice;

#include "engine/rendering/vulkan/named/igvulkan_named_set_layout.h"

#include <string>

class GVulkanNamedSetLayout : public IGVulkanNamedSetLayout
{
public:
	GVulkanNamedSetLayout(IGVulkanLogicalDevice* dev,VkDescriptorSetLayout_T* layout,const char* name);

	VkDescriptorSetLayout_T* get_layout() const noexcept override;

	void destroy() override;
private:
	std::string m_name;
	VkDescriptorSetLayout_T* m_layout;
	IGVulkanLogicalDevice* m_boundedDevice;
};

#endif // GVULKAN_NAMED_SET_LAYOUT_H