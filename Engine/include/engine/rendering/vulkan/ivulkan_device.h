#ifndef IVULKAN_DEVICE
#define IVULKAN_DEVICE

#include "engine/GEngine_EXPORT.h"
#include "ivulkan_ldevice.h"
#include "ivulkan_pdevice.h"
#include "public/core/templates/shared_ptr.h"

class GVulkanCommandBuffer;

class ENGINE_API IGVulkanDevice
{
public:

	//X This is mainly for getting info about selected physical device 
	virtual GSharedPtr<IGVulkanPhysicalDevice> as_physical_device() = 0;

	//X This is mainly for creating some resources and drawing operations with selected physical device 
	virtual GSharedPtr<IGVulkanLogicalDevice> as_logical_device() = 0;

	//X Initing all device types
	virtual bool init() = 0;

	virtual bool is_valid() const = 0;

	virtual void destroy() = 0;

	virtual GVulkanCommandBuffer* get_main_command_buffer() = 0;

	virtual GVulkanCommandBuffer* get_single_time_command_buffer() = 0;

	virtual void execute_single_time_command_buffer_and_wait() = 0;
private:
};

#endif // IVULKAN_PHYSICAL_DEVICE