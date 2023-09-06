#include "internal/engine/rendering/vulkan/gvulkan_uniform_buffer.h"

GVulkanUniformBuffer::GVulkanUniformBuffer(IVulkanBuffer* buff)
{
	m_buff = buff;
}

GVulkanUniformBuffer::~GVulkanUniformBuffer()
{
	delete m_buff;
}

void GVulkanUniformBuffer::unload()
{
	m_buff->unload();
}

IGVulkanLogicalDevice* GVulkanUniformBuffer::get_bounded_device()
{
	return m_buff->get_bounded_device();
}

VkBuffer_T* GVulkanUniformBuffer::get_vk_buffer()
{
	return m_buff->get_vk_buffer();
}

VkDeviceMemory_T* GVulkanUniformBuffer::get_device_memory()
{
	return m_buff->get_device_memory();
}

void GVulkanUniformBuffer::copy_data_to_device_memory(const void* src, uint32_t size)
{
	m_buff->copy_data_to_device_memory(src, size);
}

uint32_t GVulkanUniformBuffer::get_size()
{
	return m_buff->get_size();
}
