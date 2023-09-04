#ifndef GVULKAN_VERTEX_BUFFER_H
#define GVULKAN_VERTEX_BUFFER_H

#include "engine/rendering/vulkan/igvulkan_vertex_buffer.h"


class GVulkanVertexBuffer : public IGVulkanVertexBuffer
{
public:
	GVulkanVertexBuffer(IVulkanBuffer* buff);
	~GVulkanVertexBuffer();

	// Inherited via IGVulkanVertexBuffer
	virtual void unload() override;
	virtual IGVulkanLogicalDevice* get_bounded_device() override;
	virtual VkBuffer_T* get_vk_buffer() override;
	virtual VkDeviceMemory_T* get_device_memory() override;
	virtual void copy_data_to_device_memory(void* src, uint32_t size) override;
	virtual uint32_t get_size() override;
private:
	IVulkanBuffer* m_buffer;

};

#endif // GVULKAN_VERTEX_BUFFER_H