#include "internal/engine/rendering/vulkan/vulkan_swapchain.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/vulkan_memory.h"
#include "internal/engine/rendering/vulkan/vulkan_pdevice.h"
#include <algorithm>


static VkPresentModeKHR presentMode = VK_PRESENT_MODE_MAILBOX_KHR;

GVulkanSwapchain::GVulkanSwapchain(GVulkanLogicalDevice* inDevice, VkSurfaceKHR surface, uint32_t desiredImageCount, uint32_t width, uint32_t height, VkSurfaceFormatKHR format, IGVulkanQueue* presentQueue) : m_device(inDevice),m_surface(surface)
{
	m_swapchain = nullptr;
	m_surfaceExtent.width = width;
	m_surfaceExtent.height = height;
	m_imageCount = desiredImageCount;
	m_surfaceFormat = format;
	m_presentQueue = presentQueue;
	m_currentImage = 0;
	m_needHandle = false;
}

bool GVulkanSwapchain::init()
{
	
	bool inited = get_swap_chain_support_details((VkPhysicalDevice)m_device->get_bounded_physical_device()->get_vk_physical_device(), m_surface, m_details);
	
	if (!inited)
		return false;

	if (m_details.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		m_surfaceExtent = m_details.capabilities.currentExtent;
	}
	else {
		m_surfaceExtent.width = std::clamp(m_surfaceExtent.width, m_details.capabilities.minImageExtent.width, m_details.capabilities.maxImageExtent.width);
		m_surfaceExtent.height = std::clamp(m_surfaceExtent.height, m_details.capabilities.minImageExtent.height, m_details.capabilities.maxImageExtent.height);
	}
	uint32_t presentQueue = m_presentQueue->get_queue_index();
	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.pNext = nullptr;
	info.flags = 0;
	info.minImageCount = m_imageCount;
	info.imageFormat = m_surfaceFormat.format;
	info.imageColorSpace = m_surfaceFormat.colorSpace;
	info.imageExtent = m_surfaceExtent;
	info.imageArrayLayers = 1;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.queueFamilyIndexCount = 1;
	info.pQueueFamilyIndices = &presentQueue;
	info.preTransform = m_details.capabilities.currentTransform;
	info.presentMode = presentMode;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	info.clipped = VK_TRUE;
	info.surface = m_surface;
	info.oldSwapchain = m_swapchain;

	VkResult res = vkCreateSwapchainKHR((VkDevice)m_device->get_vk_device(),&info, nullptr, &m_swapchain);
	if (res != VK_SUCCESS)
		return false;
	
	
	// Get Image and ImageView
	uint32_t imageCount = 0;
	if (vkGetSwapchainImagesKHR((VkDevice)m_device->get_vk_device(), m_swapchain, &imageCount, nullptr) != VK_SUCCESS)
	{
		return false;
	}

	m_images.resize(imageCount);
	if (vkGetSwapchainImagesKHR((VkDevice)m_device->get_vk_device(), m_swapchain, &imageCount, m_images.data()) != VK_SUCCESS)
	{
		return false;
	}

	// Create Image View

	m_imageViews.resize(imageCount);

	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = this->m_surfaceFormat.format;

	// Default Usage 
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	for (uint32_t i = 0; i < imageCount; i++)
	{

		createInfo.image = m_images[i];

		vkCreateImageView((VkDevice)m_device->get_vk_device(), &createInfo, nullptr, &(m_imageViews[i]));
	}


	std::vector<C_GVec2> sizes(m_images.size());
	for (int i = 0; i < sizes.size(); i++)
	{
		sizes[i].x = m_surfaceExtent.width;
		sizes[i].y = m_surfaceExtent.height;
	}

	std::vector<VkClearValue> clearValues;
	clearValues.push_back({ {0.f,0.f,0.f,0.f} });

	m_renderpass.create((VkDevice)m_device->get_vk_device(), m_imageViews, sizes, clearValues, this->m_surfaceFormat.format,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	
	m_needHandle = false;
	return !m_renderpass.is_failed();

}

void GVulkanSwapchain::destroy()
{
	m_renderpass.destroy((VkDevice)m_device->get_vk_device());

	for (int i = 0; i < m_imageViews.size(); i++)
	{
		vkDestroyImageView((VkDevice)m_device->get_vk_device(), m_imageViews[i], nullptr);
	}
	vkDestroySwapchainKHR((VkDevice)m_device->get_vk_device(),m_swapchain,nullptr);
}

GVulkanRenderpass* GVulkanSwapchain::get_renderpass()
{
	return &m_renderpass;
}

bool GVulkanSwapchain::acquire_draw_image(GVulkanSemaphore* semaphore)
{
	VkResult res = vkAcquireNextImageKHR((VkDevice)m_device->get_vk_device(), m_swapchain, UINT64_MAX,semaphore->get_semaphore(),VK_NULL_HANDLE,&m_currentImage);
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
	{
		m_needHandle = true;
		return false;
	}
	return true;
}

bool GVulkanSwapchain::present_image(uint32_t waitSemaphoreCount, GVulkanSemaphore* waitSemaphores)
{
	std::vector<VkSemaphore> semaphores(waitSemaphoreCount);
	for (int i = 0; i < waitSemaphoreCount; i++)
	{
		semaphores[i] = waitSemaphores->get_semaphore();
	}


	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = semaphores.data();
	presentInfo.waitSemaphoreCount = waitSemaphoreCount;

	presentInfo.pImageIndices = &m_currentImage;

	auto res = vkQueuePresentKHR(m_presentQueue->get_queue(), &presentInfo);
	return res == VK_SUCCESS;
}

bool GVulkanSwapchain::need_handle()
{
	return m_needHandle;
}

bool GVulkanSwapchain::handle()
{
	m_renderpass.destroy((VkDevice)m_device->get_vk_device());
	for (int i = 0; i < m_imageViews.size(); i++)
	{
		vkDestroyImageView((VkDevice)m_device->get_vk_device(), m_imageViews[i], nullptr);
	}
	init();
	return true;
}

