#include "volk.h"

#include "internal/engine/rendering/vulkan/vulkan_queue.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/vulkan_pdevice.h"

GVulkanQueue::GVulkanQueue()
{
	m_queue = nullptr;
}

GVulkanQueue::GVulkanQueue(GVulkanLogicalDevice* inDevice, uint32_t familyIndex, uint32_t queueIndex) : m_device(inDevice),m_familyIndex(familyIndex)
{
	VkDeviceQueueInfo2 inf = {};
	inf.flags = 0;
	inf.queueFamilyIndex = familyIndex;
	inf.queueIndex = queueIndex;
	inf.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_INFO_2;
	inf.pNext = nullptr;

	vkGetDeviceQueue2((VkDevice)inDevice->get_vk_device(),&inf,&m_queue);

	auto queues = inDevice->get_bounded_physical_device()->get_all_queues();
	m_properties = queues[familyIndex];
	
	if (m_properties.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)
	{
		m_allQueues += "VK_QUEUE_COMPUTE_BIT|";
	}
	if (m_properties.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
	{
		m_allQueues += "VK_QUEUE_GRAPHICS_BIT|";
	}if (m_properties.queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
	{
		m_allQueues += "VK_QUEUE_TRANSFER_BIT|";
	}if (m_properties.queueFlags & VkQueueFlagBits::VK_QUEUE_SPARSE_BINDING_BIT)
	{
		m_allQueues += "VK_QUEUE_SPARSE_BINDING_BIT|";
	}if (m_properties.queueFlags & VkQueueFlagBits::VK_QUEUE_OPTICAL_FLOW_BIT_NV)
	{
		m_allQueues += "VK_QUEUE_OPTICAL_FLOW_BIT_NV|";
	}if (m_properties.queueFlags & VkQueueFlagBits::VK_QUEUE_VIDEO_DECODE_BIT_KHR)
	{
		m_allQueues += "VK_QUEUE_VIDEO_DECODE_BIT_KHR|";
	}if (m_properties.queueFlags & VkQueueFlagBits::VK_QUEUE_PROTECTED_BIT)
	{
		m_allQueues += "VK_QUEUE_PROTECTED_BIT|";
	}
	if (m_allQueues.size() != 0)
	{
		m_allQueues.pop_back();
	}
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

const char* GVulkanQueue::get_all_supported_operations_as_string() const noexcept
{
	return m_allQueues.c_str();
}
