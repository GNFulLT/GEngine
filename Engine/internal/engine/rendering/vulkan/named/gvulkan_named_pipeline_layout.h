#ifndef GVULKAN_NAMED_PIPELINE_LAYOUT_H
#define GVULKAN_NAMED_PIPELINE_LAYOUT_H

#include "volk.h"
#include "engine/rendering/vulkan/named/igvulkan_named_pipeline_layout.h"
#include <string>
#include "engine/rendering/vulkan/ivulkan_ldevice.h"


class GVulkanNamedPipelineLayout : public IGVulkanNamedPipelineLayout
{
public:
	GVulkanNamedPipelineLayout(IGVulkanLogicalDevice* dev,const char* name,VkPipelineLayout_T* layout);

	// Inherited via IGVulkanNamedPipelineLayout
	virtual VkPipelineLayout_T* get_vk_pipeline_layout() const noexcept override;
	virtual void destroy() override;
private:
	VkPipelineLayout_T* m_layout;
	std::string m_name;
	IGVulkanLogicalDevice* p_device;
};


#endif // GVULKAN_NAMED_PIPELINE_LAYOUT_H