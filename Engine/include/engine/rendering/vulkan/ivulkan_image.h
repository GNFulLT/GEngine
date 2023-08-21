#ifndef IVULKAN_IMAGE_H
#define IVULKAN_IMAGE_H

#include "engine/GEngine_EXPORT.h"

class IGVulkanLogicalDevice;
struct VkImageViewCreateInfo;
struct VkImage_T;
class ENGINE_API IVulkanImage
{
public:
	virtual ~IVulkanImage() = default;

	virtual void unload() = 0;

	virtual IGVulkanLogicalDevice* get_bounded_device() = 0;
	
	// You don't need to fill the image property. In this method it will be filled automatically 
	virtual bool create_image_view(const VkImageViewCreateInfo* info) = 0;

	virtual VkImage_T* get_vk_image() = 0;
private:
};

#endif // IVULKAN_IMAGE_H