#ifndef IVULKAN_BUFFER_H
#define IVULKAN_BUFFER_H

#include "engine/GEngine_EXPORT.h"

class IGVulkanLogicalDevice;

class ENGINE_API IVulkanBuffer
{
public:
	virtual ~IVulkanBuffer() = default;

	virtual void unload() = 0;

	virtual IGVulkanLogicalDevice* get_bounded_device() = 0;
private:
};

#endif // IVULKAN_BUFFER_H