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
}

GVulkanImage::~GVulkanImage()
{
	if (m_inited)
	{
		GLoggerManager::get_instance()->log_e("GVulkanImage","Forgot to uninitialize image.");
	}
}

void GVulkanImage::unload()
{
	if (m_inited)
	{
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
