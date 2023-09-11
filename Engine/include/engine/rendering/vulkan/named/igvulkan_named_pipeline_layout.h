#ifndef IGVULKAN_NAMED_PIPELINE_LAYOUT_H
#define IGVULKAN_NAMED_PIPELINE_LAYOUT_H

#include "engine/rendering/vulkan/ivulkan_renderpass.h"

struct VkPipelineLayout_T;

class IGVulkanNamedPipelineLayout
{
public:
	virtual ~IGVulkanNamedPipelineLayout() = default;
	virtual VkPipelineLayout_T* get_vk_pipeline_layout() const noexcept= 0;
	virtual VkDescriptorSetLayout_T* get_vk_pipeline_set_layout() const noexcept = 0;

	virtual void destroy() = 0;
private:

};

#endif // IGVULKAN_NAMED_PIPELINE_LAYOUT_H