#ifndef IVULKAN_DEVICE
#define IVULKAN_DEVICE

#include "engine/GEngine_EXPORT.h"
#include "ivulkan_ldevice.h"
#include "ivulkan_pdevice.h"
#include "public/core/templates/shared_ptr.h"

#include <expected>

class ITransferHandle;
enum TRANSFER_QUEUE_GET_ERR;
class GVulkanCommandBuffer;
class GVulkanSemaphore;
class GVulkanFence;
struct VkSubmitInfo;

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

	// DEPRECATED
	virtual void execute_single_time_command_buffer_and_wait() = 0;	

	virtual GVulkanCommandBuffer* create_cmd_from_main_pool() = 0;
	virtual void destroy_cmd_main_pool(GVulkanCommandBuffer* cmd) = 0;

	virtual GVulkanSemaphore* create_semaphore(bool isSignaled) = 0;

	virtual void add_wait_semaphore_for_this_frame(GVulkanSemaphore* semaphore,int pipelineStageFlag) = 0;

	virtual void destroy_semaphore(GVulkanSemaphore* semaphore) = 0;

	virtual void execute_cmd_from_main(GVulkanCommandBuffer* buff, const VkSubmitInfo* inf, GVulkanFence* fence) = 0;

private:
};

#endif // IVULKAN_PHYSICAL_DEVICE