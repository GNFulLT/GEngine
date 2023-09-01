#ifndef IVULKAN_PIPELINE_LAYOUT_H
#define IVULKAN_PIPELINE_LAYOUT_H

#include "engine/GEngine_EXPORT.h"

struct VkPipelineLayout_T;

class ENGINE_API IGVulkanPipelineLayout
{
public:
	virtual ~IGVulkanPipelineLayout() = default;

	virtual VkPipelineLayout_T* get_vk_pipeline_layout() = 0;

	virtual void destroy() = 0;
private:
};

#endif // IVULKAN_PIPELINE_LAYOUT_H