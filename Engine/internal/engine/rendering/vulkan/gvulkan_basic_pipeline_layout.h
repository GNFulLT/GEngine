#ifndef GVULKAN_BASIC_PIPELINE_LAYOUT_H
#define GVULKAN_BASIC_PIPELINE_LAYOUT_H

#include "engine/rendering/vulkan/ivulkan_pipeline_layout.h"

class IGVulkanLogicalDevice;

class GVulkanBasicPipelineLayout : public IGVulkanPipelineLayout
{
public:
	GVulkanBasicPipelineLayout(IGVulkanLogicalDevice* dev, VkPipelineLayout_T* layout);
	// Inherited via IGVulkanPipelineLayout
	virtual VkPipelineLayout_T* get_vk_pipeline_layout() override;
	virtual void destroy() override;
private:
	VkPipelineLayout_T* m_layout;
	IGVulkanLogicalDevice* m_boundedDevice;
};

#endif // GVULKAN_PIPELINE_LAYOUT_WRAPPER_H