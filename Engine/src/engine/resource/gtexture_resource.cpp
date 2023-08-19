#include "internal/engine/resource/gtexture_resource.h"
#include "engine/io/iimage_loader.h"
#include <volk.h>
#include <vma/vk_mem_alloc.h>
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_queue.h"
VkImageType gtype_to_image_type(GIMAGETYPE type)
{
	switch (type)
	{
	case GIMAGETYPE_2D:
		return VK_IMAGE_TYPE_2D;
	case GIMAGETYPE_CUBEMAP:
		return VK_IMAGE_TYPE_3D;
	default:
		return VK_IMAGE_TYPE_2D;
	}
}


GTextureResource::GTextureResource(std::string_view filePath, IImageLoader* loader, IGVulkanLogicalDevice* parentDevice)
{	
	m_filePath = filePath;
	m_loader = loader;
	m_boundedDevice = parentDevice;
	m_gpuBuffer = nullptr;
}

RESOURCE_INIT_CODE GTextureResource::prepare_impl()
{
	if (m_loader == nullptr)
		return RESOURCE_INIT_CODE::RESOURCE_INIT_CODE_UNKNOWN_EX;

	m_imageDescriptor = m_loader->load(m_filePath);
	
	if (m_imageDescriptor == nullptr)
		return RESOURCE_INIT_CODE::RESOURCE_INIT_CODE_FILE_NOT_FOUND;

	return RESOURCE_INIT_CODE_OK;
}

void GTextureResource::unprepare_impl()
{
}

RESOURCE_INIT_CODE GTextureResource::load_impl()
{
	calculateSize();
	
	VkExtent3D imageExtent;
	imageExtent.width = static_cast<uint32_t>(m_imageDescriptor->width);
	imageExtent.height = static_cast<uint32_t>(m_imageDescriptor->height);
	imageExtent.depth = 1;

	uint32_t index = m_boundedDevice->get_render_queue()->get_queue_index();

	VkImageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.imageType = gtype_to_image_type(m_imageDescriptor->imageType);
	info.format = m_imageDescriptor->format;
	info.extent = imageExtent;
	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.queueFamilyIndexCount =1;
	info.pQueueFamilyIndices = &index;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// First create memory for the image
	// After than map the image to the gpu

	auto res = m_boundedDevice->create_image(&info, VMA_MEMORY_USAGE_CPU_ONLY);



	return RESOURCE_INIT_CODE_OK;
}

void GTextureResource::unload_impl()
{
}

std::uint64_t GTextureResource::calculateSize() const
{
	if (m_imageDescriptor == nullptr)
	{
		m_size = 0;
	}
	else
	{
		m_size = m_imageDescriptor->channelCount* m_imageDescriptor->width* m_imageDescriptor->height;
	}
	return m_size;
}

std::string_view GTextureResource::get_resource_path() const
{
	return m_filePath;
}


IImageLoader* IGTextureResource::get_loader()
{
	return m_loader;
}
