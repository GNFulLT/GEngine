#ifndef IVULKAN_L_DEVICE_H
#define IVULKAN_L_DEVICE_H

#include "engine/GEngine_EXPORT.h"
#include <expected>
#include <cstdint>

class IGVulkanPhysicalDevice;
class IGVulkanQueue;
class GVulkanCommandBuffer;
class IVulkanBuffer;
class IVulkanImage;

struct VkImageCreateInfo;

enum VkFormat;

enum VULKAN_BUFFER_CREATION_ERROR
{
	VULKAN_BUFFER_CREATION_ERROR_UNKNOWN
};

enum VULKAN_IMAGE_CREATION_ERROR
{
	VULKAN_IMAGE_CREATION_ERROR_UNKNOWN
};

enum VmaMemoryUsage;

class ENGINE_API IGVulkanLogicalDevice
{
public:
	virtual ~IGVulkanLogicalDevice() = default;

	virtual bool init() = 0;

	virtual bool is_valid() const = 0;

	virtual void destroy() = 0;

	virtual void* get_vk_device() = 0;

	virtual IGVulkanPhysicalDevice* get_bounded_physical_device() = 0;

	virtual IGVulkanQueue* get_present_queue() = 0;

	virtual IGVulkanQueue* get_render_queue() = 0;

	virtual IGVulkanQueue* get_resource_queue() = 0;

	virtual bool begin_command_buffer_record(GVulkanCommandBuffer* buff) = 0;

	virtual void end_command_buffer_record(GVulkanCommandBuffer* buff) = 0;


	virtual std::expected<IVulkanBuffer*, VULKAN_BUFFER_CREATION_ERROR> create_buffer(uint64_t size,uint64_t usageFlag, VmaMemoryUsage memoryUsageFlag) = 0;


	virtual std::expected< IVulkanImage*, VULKAN_IMAGE_CREATION_ERROR> create_image(const VkImageCreateInfo* imageCreateInfo, VmaMemoryUsage memoryUsageFlag) = 0;
private:
};


#endif // IVULKAN_L_DEVICE_H