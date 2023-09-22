#ifndef GVULKAN_INDIRECT_BUFFER_H
#define GVULKAN_INDIRECT_BUFFER_H

#include "engine/rendering/vulkan/igvulkan_indirect_buffer.h"
#include <vma/vk_mem_alloc.h>

class GVulkanIndirectBuffer : public IGVulkanIndirectBuffer
{
public:
	GVulkanIndirectBuffer(IVulkanBuffer* buff);
	~GVulkanIndirectBuffer();
	// Inherited via IGVulkanUniformBuffer
	virtual void unload() override;
	virtual IGVulkanLogicalDevice* get_bounded_device() override;
	virtual VkBuffer_T* get_vk_buffer() override;
	virtual VkDeviceMemory_T* get_device_memory() override;
	virtual void copy_data_to_device_memory(const void* src, uint32_t size) override;

	virtual uint32_t get_size() override;

private:
	IVulkanBuffer* m_buff;


	// Inherited via IGVulkanIndirectBuffer
	virtual void* map_memory() override;

	virtual void unmap_memory() override;

};

#endif // GVULKAN_INDIRECT_BUFFER_H