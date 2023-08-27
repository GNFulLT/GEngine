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
class IGVulkanDevice;

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
class ITransferHandle;
enum TRANSFER_QUEUE_GET_ERR;
struct VkDevice_T;
class ENGINE_API IGVulkanLogicalDevice
{
public:
	virtual ~IGVulkanLogicalDevice() = default;

	virtual bool init() = 0;

	virtual bool is_valid() const = 0;

	virtual void destroy() = 0;

	virtual VkDevice_T* get_vk_device() = 0;

	virtual IGVulkanPhysicalDevice* get_bounded_physical_device() = 0;

	virtual IGVulkanQueue* get_present_queue() noexcept = 0;

	virtual IGVulkanQueue* get_render_queue() noexcept = 0;

	virtual IGVulkanQueue* get_resource_queue() noexcept = 0;

	virtual bool begin_command_buffer_record(GVulkanCommandBuffer* buff) = 0;

	virtual void end_command_buffer_record(GVulkanCommandBuffer* buff) = 0;


	virtual std::expected<IVulkanBuffer*, VULKAN_BUFFER_CREATION_ERROR> create_buffer(uint64_t size, uint32_t bufferUsageFlags, VmaMemoryUsage memoryUsageFlag) = 0;


	virtual std::expected< IVulkanImage*, VULKAN_IMAGE_CREATION_ERROR> create_image(const VkImageCreateInfo* imageCreateInfo, VmaMemoryUsage memoryUsageFlag) = 0;

	virtual IGVulkanDevice* get_owner() noexcept = 0;


	virtual std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> get_wait_and_begin_transfer_cmd() = 0;

	virtual std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> get_wait_and_begin_transfer_cmd(uint64_t timeout) = 0;

	virtual void finish_execute_and_wait_transfer_cmd(ITransferHandle* handle) = 0;
private:
};


#endif // IVULKAN_L_DEVICE_H