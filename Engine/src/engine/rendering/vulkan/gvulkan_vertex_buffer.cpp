#include "internal/engine/rendering/vulkan/gvulkan_vertex_buffer.h"

GVulkanVertexBuffer::GVulkanVertexBuffer(IVulkanBuffer* buff)
{
	m_buffer = buff;
}

GVulkanVertexBuffer::~GVulkanVertexBuffer()
{
	delete m_buffer;
}

void GVulkanVertexBuffer::unload()
{
	m_buffer->unload();
}

IGVulkanLogicalDevice* GVulkanVertexBuffer::get_bounded_device()
{
	return m_buffer->get_bounded_device();
}

VkBuffer_T* GVulkanVertexBuffer::get_vk_buffer()
{
	return m_buffer->get_vk_buffer();
}

VkDeviceMemory_T* GVulkanVertexBuffer::get_device_memory()
{
	return m_buffer->get_device_memory();
}

void GVulkanVertexBuffer::copy_data_to_device_memory(const void* src, uint32_t size)
{
	m_buffer->copy_data_to_device_memory(src, size);
}

uint32_t GVulkanVertexBuffer::get_size()
{
	return m_buffer->get_size();
}

void* GVulkanVertexBuffer::map_memory()
{
	return m_buffer->map_memory();
}

void GVulkanVertexBuffer::unmap_memory()
{
	m_buffer->unmap_memory();
}
