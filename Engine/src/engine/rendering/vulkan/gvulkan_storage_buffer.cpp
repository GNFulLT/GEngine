#include "internal/engine/rendering/vulkan/gvulkan_storage_buffer.h"

GVulkanStorageBuffer::GVulkanStorageBuffer(IVulkanBuffer* buff)
{
	m_buff = buff;
}

GVulkanStorageBuffer::~GVulkanStorageBuffer()
{
	delete m_buff;
}

void GVulkanStorageBuffer::unload()
{
	m_buff->unload();
}

IGVulkanLogicalDevice* GVulkanStorageBuffer::get_bounded_device()
{
	return m_buff->get_bounded_device();
}

VkBuffer_T* GVulkanStorageBuffer::get_vk_buffer()
{
	return m_buff->get_vk_buffer();
}

VkDeviceMemory_T* GVulkanStorageBuffer::get_device_memory()
{
	return m_buff->get_device_memory();
}

void GVulkanStorageBuffer::copy_data_to_device_memory(const void* src, uint32_t size)
{
	m_buff->copy_data_to_device_memory(src, size);
}

uint32_t GVulkanStorageBuffer::get_size()
{
	return m_buff->get_size();
}

void* GVulkanStorageBuffer::map_memory()
{
	return m_buff->map_memory();
}

void GVulkanStorageBuffer::unmap_memory()
{
	m_buff->unmap_memory();
}
