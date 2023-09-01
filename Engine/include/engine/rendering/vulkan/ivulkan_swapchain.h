#ifndef IGVULKAN_SWAPCHAIN_H
#define IGVULKAN_SWAPCHAIN_H

#include "engine/GEngine_EXPORT.h"
#include <cstdint>

class ENGINE_API IGVulkanSwapchain
{
public:
	virtual ~IGVulkanSwapchain() = default;

	virtual uint32_t get_total_image() = 0;
private:
};

#endif // IGVULKAN_SWAPCHAIN_H