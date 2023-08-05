#ifndef VULKAN_VIEWPORT_H
#define VULKAN_VIEWPORT_H

#include <cstdint>
#include "engine/rendering/vulkan/ivulkan_viewport.h"

class GVulkanLogicalDevice;

class GVulkanSwapchain;
class IGVulkanQueue;
struct VkSurfaceKHR_T;
struct VkSurfaceFormatKHR;

class GVulkanViewport : public IGVulkanViewport
{
public:
	GVulkanViewport(GVulkanLogicalDevice* inDevice,uint32_t sizeX, uint32_t sizeY);
	~GVulkanViewport();

	bool init(VkSurfaceKHR_T* surface,VkSurfaceFormatKHR format,IGVulkanQueue* presentQueue);

	void destroy();

	virtual int get_current_image_index() const override;

	virtual void* get_vk_current_image_renderpass() override;

	virtual uint32_t get_width() const override;

	virtual uint32_t get_height() const override;

	virtual uint32_t get_total_image() const override;


	virtual void begin_draw_cmd(GVulkanCommandBuffer* cmd) override;
	virtual void end_draw_cmd(GVulkanCommandBuffer* cmd) override;

	virtual bool acquire_draw_image(GVulkanSemaphore* waitSemaphore) override;
	
	virtual bool present_image(uint32_t waitSemaphoreCount, GVulkanSemaphore* waitSemaphores) override;


	virtual bool need_handle() override;

	virtual bool handle() override;
private:
	GVulkanLogicalDevice* m_device;
private:
	GVulkanSwapchain* m_swapchain;
	uint32_t m_sizeX;
	uint32_t m_sizeY;
};

#endif //VULKAN_VIEWPORT_H