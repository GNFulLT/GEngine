#ifndef IGVULKAN_CHAINED_VIEWPORT_H
#define IGVULKAN_CHAINED_VIEWPORT_H

#include "engine/rendering/vulkan/ivulkan_viewport.h"

class ENGINE_API IGVulkanChainedViewport : public IGVulkanViewport
{
public:
	virtual ~IGVulkanChainedViewport() = default;

	virtual void set_image_index(uint32_t index) = 0;

	virtual uint32_t get_image_count() = 0;
private:
};

#endif // IGVULKAN_CHAINED_VIEWPORT_H