#ifndef GRAPHIC_DEVICE_H
#define GRAPHIC_DEVICE_H

#include "public/core/templates/shared_ptr.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include <mutex>
#include <expected>

class IGVulkanApp;
class GVulkanCommandBufferManager;
class GVulkanCommandBuffer;
class GVulkanFence;
class GVulkanFenceManager;
class GVulkanSemaphore;
class GVulkanSemaphoreManager;

#include <queue>

typedef std::pair< GVulkanCommandBuffer*, uint32_t> transfer_handle;

enum TRANSFER_QUEUE_MODE
{
	TRANSFER_QUEUE_MODE_DEFAULT,
	TRANSFER_QUEUE_MODE_TRANSFER
};

enum TRANSFER_QUEUE_GET_ERR
{
	TRANSFER_QUEUE_GET_ERR_TIMEOUT
};	

class GVulkanDevice : public IGVulkanDevice
{
public:
	GVulkanDevice(GWeakPtr<IGVulkanApp> vulkanApp);

	~GVulkanDevice();
	//X This is mainly for getting info about selected physical device 
	virtual GSharedPtr<IGVulkanPhysicalDevice> as_physical_device() override;

	//X This is mainly for creating some resources and drawing operations with selected physical device 
	virtual GSharedPtr<IGVulkanLogicalDevice> as_logical_device() override;

	//X Initing all device types
	virtual bool init() override;

	virtual bool is_valid() const override;

	virtual void destroy() override;

	virtual GVulkanCommandBuffer* get_main_command_buffer() override;

	virtual GVulkanCommandBuffer* get_single_time_command_buffer() override;

	virtual void execute_single_time_command_buffer_and_wait() override;

	bool prepare_for_rendering();

	GVulkanSemaphore* get_image_acquired_semaphore();
	GVulkanSemaphore* get_render_complete_semaphore();
	GVulkanFence* get_queue_fence();


	

private:
	bool reset_things();



private:
	GSharedPtr<IGVulkanPhysicalDevice> m_vulkanPhysicalDevice;
	GSharedPtr<IGVulkanLogicalDevice> m_vulkanLogicalDevice;
	GWeakPtr<IGVulkanApp> m_vulkanApp;
	bool is_inited;
	bool m_destroyed;

	GSharedPtr<GVulkanFenceManager> m_fenceManager;
	GVulkanFence* m_renderingFence;

	GSharedPtr<GVulkanSemaphoreManager> m_semaphoreManager;
	GVulkanSemaphore* m_imageAcquiredSemaphore;
	GVulkanSemaphore* m_presentationSemaphore;

	GSharedPtr<GVulkanCommandBufferManager> m_defaultCommandManager;
	GVulkanCommandBuffer* m_mainCommandBuffer;
	GVulkanCommandBuffer* m_singleTimeCommandBuffer;


	TRANSFER_QUEUE_MODE m_transerMode;
};

#endif // GRAPHIC_DEVICE_H