#ifndef IGVULKAN_DESCRIPTOR_POOL_H
#define IGVULKAN_DESCRIPTOR_POOL_H

#include "engine/GEngine_EXPORT.h"


struct VkDescriptorPool_T;

class ENGINE_API IGVulkanDescriptorPool
{
public:
	virtual ~IGVulkanDescriptorPool() = default;
	
	virtual VkDescriptorPool_T* get_vk_descriptor_pool() = 0;

	virtual void destroy() = 0;
private:
};

#endif // IGVULKAN_DESCRIPTOR_POOL_H