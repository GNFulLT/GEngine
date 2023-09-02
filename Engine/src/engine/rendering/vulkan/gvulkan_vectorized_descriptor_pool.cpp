#include "internal/engine/rendering/vulkan/gvulkan_vectorized_descriptor_pool.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"

GVulkanVectorizedDescriptorPool::GVulkanVectorizedDescriptorPool(IGVulkanLogicalDevice* dev,uint32_t imageCount,const std::unordered_map<VkDescriptorType, int>& typeMap)
{
	m_imageCount = imageCount;
	m_boundedDevice = dev;

	for (auto pair : typeMap)
	{
		m_poolSizes.push_back(VkDescriptorPoolSize{ .type = pair.first,.descriptorCount = m_imageCount * pair.second });
	}
}

bool GVulkanVectorizedDescriptorPool::init()
{
	const VkDescriptorPoolCreateInfo pi = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,    .pNext = nullptr, .flags = 0,    .maxSets = static_cast<uint32_t>(m_imageCount), .poolSizeCount = static_cast<uint32_t>(m_poolSizes.size()),    .pPoolSizes = m_poolSizes.empty() ? nullptr : m_poolSizes.data() };
	auto res = vkCreateDescriptorPool(m_boundedDevice->get_vk_device(), &pi, nullptr, &m_pool);
	return res == VK_SUCCESS;
}

VkDescriptorPool_T* GVulkanVectorizedDescriptorPool::get_vk_descriptor_pool()
{
	return m_pool;
}

void GVulkanVectorizedDescriptorPool::destroy()
{
	if (m_pool != nullptr)
	{
		vkDestroyDescriptorPool(m_boundedDevice->get_vk_device(), m_pool, nullptr);
	}
}
