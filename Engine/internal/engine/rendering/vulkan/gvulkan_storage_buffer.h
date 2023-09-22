#ifndef GVULKAN_STORAGE_BUFFER_H
#define GVULKAN_STORAGE_BUFFER_H

#include "engine/rendering/vulkan/igvulkan_storage_buffer.h"
#include <vma/vk_mem_alloc.h>

class GVulkanStorageBuffer : public IGVulkanStorageBuffer
{
public:
	GVulkanStorageBuffer(IVulkanBuffer* buff);
	~GVulkanStorageBuffer();
	// Inherited via IGVulkanUniformBuffer
	virtual void unload() override;
	virtual IGVulkanLogicalDevice* get_bounded_device() override;
	virtual VkBuffer_T* get_vk_buffer() override;
	virtual VkDeviceMemory_T* get_device_memory() override;
	virtual void copy_data_to_device_memory(const void* src, uint32_t size) override;

	virtual uint32_t get_size() override;

private:
	IVulkanBuffer* m_buff;


	// Inherited via IGVulkanStorageBuffer
	virtual void* map_memory() override;

	virtual void unmap_memory() override;

};

#endif // GVULKAN_STORAGE_BUFFER_H