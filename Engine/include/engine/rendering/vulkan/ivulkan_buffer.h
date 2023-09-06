#ifndef IVULKAN_BUFFER_H
#define IVULKAN_BUFFER_H

#include "engine/GEngine_EXPORT.h"
#include <cstdint>

class IGVulkanLogicalDevice;

struct VkBuffer_T;
struct VkDeviceMemory_T;

class ENGINE_API IVulkanBuffer
{
public:
	virtual ~IVulkanBuffer() = default;

	virtual void unload() = 0;

	virtual IGVulkanLogicalDevice* get_bounded_device() = 0;
	
	virtual VkBuffer_T* get_vk_buffer() = 0;

	virtual VkDeviceMemory_T* get_device_memory() = 0;

	virtual void copy_data_to_device_memory(const void* src,uint32_t size) = 0;

	virtual uint32_t get_size() = 0;

private:
};

#endif // IVULKAN_BUFFER_H