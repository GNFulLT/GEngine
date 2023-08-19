#ifndef IVULKAN_IMAGE_H
#define IVULKAN_IMAGE_H

#include "engine/GEngine_EXPORT.h"

class IGVulkanLogicalDevice;

class ENGINE_API IVulkanImage
{
public:
	virtual ~IVulkanImage() = default;

	virtual void unload() = 0;

	virtual IGVulkanLogicalDevice* get_bounded_device() = 0;
private:
};

#endif // IVULKAN_IMAGE_H