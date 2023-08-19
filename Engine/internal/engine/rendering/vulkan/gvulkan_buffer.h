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


	// Inherited via IVulkanBuffer
	virtual void unload() override;
	virtual IGVulkanLogicalDevice* get_bounded_device() override;
private:
	GVulkanLogicalDevice* m_boundedDevice;
	VmaAllocator m_allocatorRef;
	VmaAllocation m_allocationBlock;
	VkBuffer m_buffer;
	bool m_inited;

};

#endif // GVULKAN_BUFFER_H