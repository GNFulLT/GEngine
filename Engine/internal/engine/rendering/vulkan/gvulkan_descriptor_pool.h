#ifndef GVULKAN_DESCRIPTOR_POOL_H
#define GVULKAN_DESCRIPTOR_POOL_H

#include "engine/rendering/vulkan/ivulkan_descriptor_pool.h"
#include <cstdint>

class IGVulkanLogicalDevice;
class GVulkanDescriptorPool : public IGVulkanDescriptorPool
{
public:
	GVulkanDescriptorPool(IGVulkanLogicalDevice* dev,uint32_t imageCount,uint32_t uniformBufferCount, uint32_t storageBufferCount, uint32_t samplerCount);

	bool init();

	// Inherited via IGVulkanDescriptorPool
	virtual VkDescriptorPool_T* get_vk_descriptor_pool() override;
	virtual void destroy() override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	VkDescriptorPool_T* m_pool;
	uint32_t m_imageCount;
	uint32_t m_uniformBufferCount;
	uint32_t m_storageBufferCount;
	uint32_t m_samplerCount;

};

#endif // GVULKAN_DESCRIPTOR_POOL_H