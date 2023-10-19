#include <volk.h>

#include "internal/engine/rendering/vulkan/gvulkan_buffer.h"
#include "internal/engine/manager/glogger_manager.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"

#include <vma/vk_mem_alloc.h>

GVulkanBuffer::GVulkanBuffer(GVulkanLogicalDevice* owner, VmaAllocator allocator,uint32_t size)
{
	m_inited = false;
	m_allocationBlock = nullptr;
	m_allocatorRef = allocator;
	m_boundedDevice = owner;
	m_buffer = nullptr;
	m_size = size;
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

VkBuffer_T* GVulkanBuffer::get_vk_buffer()
{
	return m_buffer;
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

VkDeviceMemory_T* GVulkanBuffer::get_device_memory()
{
	VmaAllocationInfo inf = {};
	vmaGetAllocationInfo(m_allocatorRef, m_allocationBlock, &inf);
	return inf.deviceMemory;
}

void GVulkanBuffer::copy_data_to_device_memory(const void* src, uint32_t size)
{
	//X TODO : CHECK AND SHOULD RETURN RESPONSE
	void* data = nullptr;
	vmaMapMemory(m_allocatorRef, m_allocationBlock, &data);
	memcpy(data, src, size);
	vmaUnmapMemory(m_allocatorRef, m_allocationBlock);
}

uint32_t GVulkanBuffer::get_size()
{
	return m_size;
}


void* GVulkanBuffer::map_memory()
{
	void* data = nullptr;
	vmaMapMemory(m_allocatorRef, m_allocationBlock, &data);
	return data;
}

void GVulkanBuffer::unmap_memory()
{
	vmaUnmapMemory(m_allocatorRef, m_allocationBlock);
}

void GVulkanBuffer::set_name(const char* name)
{
	vmaSetAllocationName(m_allocatorRef, m_allocationBlock, name);
}