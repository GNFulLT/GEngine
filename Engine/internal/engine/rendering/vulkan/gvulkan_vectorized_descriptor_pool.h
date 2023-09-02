#ifndef GVULKAN_VECTORIZED_DESCRIPTOR_POOL_H
#define GVULKAN_VECTORIZED_DESCRIPTOR_POOL_H

#include <volk.h>
#include "engine/rendering/vulkan/ivulkan_descriptor_pool.h"
#include <cstdint>
#include <vector>
#include <unordered_map>

class IGVulkanLogicalDevice;
class GVulkanVectorizedDescriptorPool : public IGVulkanDescriptorPool
{
public:
	GVulkanVectorizedDescriptorPool(IGVulkanLogicalDevice* dev, uint32_t imageCount, const std::unordered_map<VkDescriptorType, int>& typeMap);

	bool init();
	// Inherited via IGVulkanDescriptorPool
	virtual VkDescriptorPool_T* get_vk_descriptor_pool() override;
	virtual void destroy() override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	uint32_t m_imageCount;
	std::vector<VkDescriptorPoolSize> m_poolSizes;
	VkDescriptorPool_T* m_pool;

};

#endif // GVULKAN_VECTORIZED_DESCRIPTOR_POOL_H