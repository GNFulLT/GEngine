#include "internal/engine/rendering/vulkan/gvulkan_indirect_buffer.h"

GVulkanIndirectBuffer::GVulkanIndirectBuffer(IVulkanBuffer* buff)
{
	m_buff = buff;
}

GVulkanIndirectBuffer::~GVulkanIndirectBuffer()
{
	delete m_buff;
}

void GVulkanIndirectBuffer::unload()
{
	m_buff->unload();
}

IGVulkanLogicalDevice* GVulkanIndirectBuffer::get_bounded_device()
{
	return m_buff->get_bounded_device();
}

VkBuffer_T* GVulkanIndirectBuffer::get_vk_buffer()
{
	return m_buff->get_vk_buffer();
}

VkDeviceMemory_T* GVulkanIndirectBuffer::get_device_memory()
{
	return m_buff->get_device_memory();
}

void GVulkanIndirectBuffer::copy_data_to_device_memory(const void* src, uint32_t size)
{
	m_buff->copy_data_to_device_memory(src, size);
}

uint32_t GVulkanIndirectBuffer::get_size()
{
	return m_buff->get_size();
}

void* GVulkanIndirectBuffer::map_memory()
{
	return m_buff->map_memory();
}

void GVulkanIndirectBuffer::unmap_memory()
{
	m_buff->unmap_memory();
}
