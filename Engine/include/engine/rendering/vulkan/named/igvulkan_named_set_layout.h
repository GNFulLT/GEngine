#ifndef IGVULKAN_NAMED_SET_LAYOUT_H
#define IGVULKAN_NAMED_SET_LAYOUT_H


#include "engine/GEngine_EXPORT.h"

struct VkDescriptorSetLayout_T;


class ENGINE_API IGVulkanNamedSetLayout
{
public:
	virtual ~IGVulkanNamedSetLayout() = default;
	
	virtual VkDescriptorSetLayout_T* get_layout() const noexcept = 0;

	virtual void destroy() = 0;

private:
};

#endif // IGVULKAN_NAMED_SET_LAYOUT_H