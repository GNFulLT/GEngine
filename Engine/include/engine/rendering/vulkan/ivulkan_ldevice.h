#ifndef IVULKAN_L_DEVICE_H
#define IVULKAN_L_DEVICE_H

#include "engine/GEngine_EXPORT.h"

class IGVulkanPhysicalDevice;
class IGVulkanQueue;
class GVulkanCommandBuffer;

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

	virtual bool begin_command_buffer_record(GVulkanCommandBuffer* buff) = 0;

	virtual void end_command_buffer_record(GVulkanCommandBuffer* buff) = 0;
private:
};


#endif // IVULKAN_L_DEVICE_H