#include "volk.h"

#include "internal/engine/rendering/vulkan/gvulkan_image.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include "internal/engine/manager/glogger_manager.h"

GVulkanImage::GVulkanImage(GVulkanLogicalDevice* owner, VmaAllocator allocator)
{
	m_allocationBlock = nullptr;
	m_boundedDevice = owner;
	m_allocator = allocator;
	m_inited = false;
	m_creationInfo = {};
	m_image = nullptr;
	m_imageView = nullptr;
	m_imageViewCreationInfo = {};
}

GVulkanImage::~GVulkanImage()
{
	if (m_inited)
	{
		GLoggerManager::get_instance()->log_e("GVulkanImage","Forgot to uninitialize image.");
	}
}

//X TODO : ADD DELETER
void GVulkanImage::unload()
{
	if (m_inited)
	{
		if (m_imageView != nullptr)
		{
			vkDestroyImageView((VkDevice)m_boundedDevice->get_vk_device(), m_imageView, nullptr);
		}
		vmaDestroyImage(m_allocator, m_image, m_allocationBlock);
		m_inited = false;
	}
}

VkImage* GVulkanImage::get_pimage()
{
	return &m_image;
}

VmaAllocation* GVulkanImage::get_pallocation()
{
	return &m_allocationBlock;
}

IGVulkanLogicalDevice* GVulkanImage::get_bounded_device()
{
	return m_boundedDevice;
}

bool GVulkanImage::create_image_view(const VkImageViewCreateInfo* info)
{
	if (m_imageView != nullptr)
	{
		vkDestroyImageView((VkDevice)m_boundedDevice->get_vk_device(), m_imageView, nullptr);
	}

	m_imageViewCreationInfo = *info;
	m_imageViewCreationInfo.image = m_image;
	auto dev = (VkDevice)m_boundedDevice->get_vk_device();
	auto res = vkCreateImageView(dev, &m_imageViewCreationInfo, nullptr, &m_imageView);
	return res == VK_SUCCESS;
}

VkImage_T* GVulkanImage::get_vk_image()
{
	return m_image;
}

VkImageView_T* GVulkanImage::get_vk_image_view()
{
	return m_imageView;
}

void GVulkanImage::set_image_view(VkImageView_T* view, const VkImageViewCreateInfo* inf)
{
	if (m_imageView != nullptr)
	{
		vkDestroyImageView((VkDevice)m_boundedDevice->get_vk_device(), m_imageView, nullptr);
	}

	m_imageViewCreationInfo = *inf;
	m_imageView = view;
}
