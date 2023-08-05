#include "internal/engine/rendering/vulkan/vulkan_queue.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/vulkan_pdevice.h"
#include "volk.h"

GVulkanQueue::GVulkanQueue()
{
	m_queue = nullptr;
}

GVulkanQueue::GVulkanQueue(GVulkanLogicalDevice* inDevice, uint32_t familyIndex, uint32_t queueIndex) : m_device(inDevice),m_familyIndex(familyIndex)
{
	vkGetDeviceQueue((VkDevice)inDevice->get_vk_device(),familyIndex,queueIndex,&m_queue);
}

VkQueue_T* GVulkanQueue::get_queue() const noexcept
{
	return m_queue;
}

bool GVulkanQueue::is_valid() const
{
	return m_queue != nullptr;
}

uint32_t GVulkanQueue::get_queue_index() const noexcept
{
	return m_familyIndex;
}
