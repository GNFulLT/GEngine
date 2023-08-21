#ifndef GVULKAN_BUFFER_H
#define GVULKAN_BUFFER_H

#include "engine/rendering/vulkan/ivulkan_buffer.h"
#include <vma/vk_mem_alloc.h>

class GVulkanLogicalDevice;

class GVulkanBuffer : public IVulkanBuffer
{
	friend class GVulkanLogicalDevice;
public:
	GVulkanBuffer(GVulkanLogicalDevice* owner, VmaAllocator allocator);
	~GVulkanBuffer();

	VmaAllocation* get_allocation();
	VkBuffer* get_buffer_pptr();

	virtual VkBuffer_T* get_vk_buffer() override;

	// Inherited via IVulkanBuffer
	virtual void unload() override;
	virtual IGVulkanLogicalDevice* get_bounded_device() override;

	virtual VkDeviceMemory_T* get_device_memory() override;

	virtual void copy_data_to_device_memory(void* src, uint32_t size) override;
private:
	GVulkanLogicalDevice* m_boundedDevice;
	VmaAllocator m_allocatorRef;
	VmaAllocation m_allocationBlock;
	VkBuffer m_buffer;
	bool m_inited;

};

#endif // GVULKAN_BUFFER_H