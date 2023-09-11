#ifndef GVULKAN_PIPELINE_LAYOUT_REF_H
#define GVULKAN_PIPELINE_LAYOUT_REF_H


#include "engine/rendering/vulkan/ivulkan_pipeline_layout.h"

class GVulkanPipelineLayoutRef : public IGVulkanPipelineLayout
{
public:
	GVulkanPipelineLayoutRef(VkPipelineLayout_T* layout);

	virtual VkPipelineLayout_T* get_vk_pipeline_layout() override;
	virtual void destroy() override;
private:
	VkPipelineLayout_T* m_layout;
};

#endif // GVULKAN_PIPELINE_LAYOUT_REF_H