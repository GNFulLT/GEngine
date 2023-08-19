#include "internal/engine/rendering/vulkan/gvulkan_buffer.h"
#include "internal/engine/manager/glogger_manager.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"

GVulkanBuffer::GVulkanBuffer(GVulkanLogicalDevice* owner, VmaAllocator allocator)
{
	m_inited = false;
	m_allocationBlock = nullptr;

	m_allocatorRef = allocator;
	m_boundedDevice = owner;
}

GVulkanBuffer::~GVulkanBuffer()
{
	if (m_inited)
	{
		GLoggerManager::get_instance()->log_e("GVulkanBuffer", "Forgot to uninitialize gvulkan_buffer. Hope that gpu knows about that");
	}
}

VmaAllocation* GVulkanBuffer::get_allocation()
{
	return &m_allocationBlock;
}

VkBuffer* GVulkanBuffer::get_buffer_pptr()
{
	return &m_buffer;
}

void GVulkanBuffer::unload()
{
	if (m_inited)
	{
		vmaDestroyBuffer(m_allocatorRef, m_buffer, m_allocationBlock);
		m_inited = false;
	}
}

IGVulkanLogicalDevice* GVulkanBuffer::get_bounded_device()
{
	return m_boundedDevice;
}
