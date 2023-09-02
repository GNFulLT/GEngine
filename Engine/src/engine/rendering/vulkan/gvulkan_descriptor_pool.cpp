#include "volk.h"

#include "internal/engine/rendering/vulkan/gvulkan_descriptor_pool.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include <vector>

GVulkanDescriptorPool::GVulkanDescriptorPool(IGVulkanLogicalDevice* dev, uint32_t imageCount, uint32_t uniformBufferCount, uint32_t storageBufferCount, uint32_t samplerCount)
{
	m_boundedDevice = dev;
	m_imageCount = imageCount;
	m_uniformBufferCount = uniformBufferCount;
	m_storageBufferCount = storageBufferCount;
	m_samplerCount = samplerCount;
	m_pool = nullptr;
}

bool GVulkanDescriptorPool::init()
{
	std::vector<VkDescriptorPoolSize> poolSizes;

	if (m_uniformBufferCount != 0) 
		poolSizes.push_back(VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,.descriptorCount = m_imageCount * m_uniformBufferCount});
	if (m_storageBufferCount) poolSizes.push_back(VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,       .descriptorCount = m_imageCount * m_storageBufferCount });
	if (m_samplerCount) poolSizes.push_back(VkDescriptorPoolSize{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,       .descriptorCount = m_imageCount * m_samplerCount });
	const VkDescriptorPoolCreateInfo pi = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,    .pNext = nullptr, .flags = 0,    .maxSets = static_cast<uint32_t>(m_imageCount), .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),    .pPoolSizes = poolSizes.empty() ? nullptr : poolSizes.data() };

	auto res = vkCreateDescriptorPool(m_boundedDevice->get_vk_device(), &pi,nullptr, &m_pool);
	
	return res == VK_SUCCESS;

}

VkDescriptorPool_T* GVulkanDescriptorPool::get_vk_descriptor_pool()
{
	return m_pool;
}

void GVulkanDescriptorPool::destroy()
{
	if (m_pool != nullptr)
	{
		vkDestroyDescriptorPool(m_boundedDevice->get_vk_device(), m_pool, nullptr);
	}
}
