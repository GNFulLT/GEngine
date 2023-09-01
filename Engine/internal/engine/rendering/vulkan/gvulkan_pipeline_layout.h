#ifndef GVULKAN_PIPELINE_LAYOUT_H
#define GVULKAN_PIPELINE_LAYOUT_H

#include "engine/rendering/vulkan/ivulkan_pipeline_layout.h"

class IGVulkanLogicalDevice;
struct VkDescriptorSetLayout_T;
class GVulkanPipelineLayout : public IGVulkanPipelineLayout
{
public:
	GVulkanPipelineLayout(IGVulkanLogicalDevice* dev, VkDescriptorSetLayout_T* setLayout,int flags = 0);
	bool init();
	// Inherited via IGVulkanPipelineLayout
	virtual VkPipelineLayout_T* get_vk_pipeline_layout() override;
	virtual void destroy() override;

private:
	VkPipelineLayout_T* m_layout;
	VkDescriptorSetLayout_T* m_setLayout;
	IGVulkanLogicalDevice* m_boundedDevice;
	int m_flags;
};

#endif // GVULKAN_PIPELINE_LAYOUT_H