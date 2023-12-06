#ifndef VULKAN_SWAPCHAIN_H
#define VULKAN_SWAPCHAIN_H

#include "internal/engine/rendering/vulkan/vulkan_utils.h"
#include "engine/rendering/vulkan/ivulkan_swapchain.h"
class GVulkanLogicalDevice;
class IGVulkanQueue;
class GVulkanSemaphore;
class GVulkanMainViewport;
class IGVulkanViewport;

class GVulkanSwapchain : public IGVulkanSwapchain
{
public:
	GVulkanSwapchain(GVulkanLogicalDevice* inDevice,VkSurfaceKHR surface,uint32_t desiredImageCount,uint32_t width,uint32_t height,VkSurfaceFormatKHR format,
		IGVulkanQueue* presentQueue);


	bool init();

	void destroy();

	IGVulkanViewport* get_viewport();

	inline uint32_t get_current_image_index() const
	{
		return m_currentImage;
	}

	virtual uint32_t get_total_image() override;

	bool acquire_draw_image(GVulkanSemaphore* semaphore);

	bool present_image(uint32_t waitSemaphoreCount, GVulkanSemaphore* waitSemaphores);

	bool need_handle();

	bool handle();
private:
	GVulkanMainViewport* m_viewPort;

	GVulkanLogicalDevice* m_device;

	uint32_t m_currentImage;
	VkSwapchainKHR m_swapchain;
	VkSurfaceKHR m_surface;

	std::vector<VkImage> m_images;
	std::vector<VkImageView> m_imageViews;
	VkExtent2D m_surfaceExtent;
	SwapChainSupportDetails m_details;
	uint32_t m_imageCount;
	VkSurfaceFormatKHR m_surfaceFormat;
	bool m_needHandle;
	VkPresentModeKHR m_presentMode;
	IGVulkanQueue* m_presentQueue;
	std::vector<VkPresentModeKHR> m_supportedPresentModes;

};

#endif //VULKAN_SWAPCHAIN_H