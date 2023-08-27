#include "internal/engine/resource/gtexture_resource.h"
#include "engine/io/iimage_loader.h"
#include <volk.h>
#include "internal/engine/rendering/vulkan/vma_importer.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_queue.h"
#include "engine/rendering/vulkan/ivulkan_image.h"
#include "engine/rendering/vulkan/ivulkan_buffer.h"
#include "engine/rendering/vulkan/transfer/itransfer_handle.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "engine/rendering/vulkan/ivulkan_sampler_creator.h"
#include "engine/rendering/vulkan/ivulkan_sampler.h"
#include "internal/engine/manager/glogger_manager.h"
#include "engine/rendering/vulkan/ivulkan_descriptor_creator.h"
#include "engine/manager/igresource_manager.h"
#include "engine/rendering/vulkan/transfer/itransfer_op.h"

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

VkImageViewType gtype_to_image_view_type(GIMAGETYPE type)
{
	switch (type)
	{
	case GIMAGETYPE_2D:
		return VK_IMAGE_VIEW_TYPE_2D;
	case GIMAGETYPE_CUBEMAP:
		return VK_IMAGE_VIEW_TYPE_3D;
	default:
		return VK_IMAGE_VIEW_TYPE_2D;
	}
}

GTextureResource::~GTextureResource()
{
	if (m_gpuBuffer != nullptr || m_inUsageSampler != nullptr)
	{
		GLoggerManager::get_instance()->log_c("GTextureResource","Dont forget to release Texture Resource !!!");
	}
}

GTextureResource::GTextureResource(std::string_view filePath, IImageLoader* loader, IGVulkanLogicalDevice* parentDevice, IGVulkanSamplerCreator* samplerCreator, IGVulkanDescriptorCreator* descriptorCreator)
{	
	m_filePath = filePath;
	m_loader = loader;
	m_boundedDevice = parentDevice;
	m_gpuBuffer = nullptr;
	m_samplerCreator = samplerCreator;
	m_inUsageSampler = nullptr;
	m_descriptorCreator = descriptorCreator;
	m_descriptorSet = nullptr;
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


	VkImageViewCreateInfo ivinfo = {};
	ivinfo.pNext = nullptr;
	ivinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ivinfo.format = m_imageDescriptor->format;
	ivinfo.viewType = gtype_to_image_view_type(m_imageDescriptor->imageType);
	ivinfo.flags = 0;
	ivinfo.subresourceRange.baseMipLevel = 0;
	ivinfo.subresourceRange.baseArrayLayer = 0;
	ivinfo.subresourceRange.layerCount = 1;
	ivinfo.subresourceRange.levelCount = 1;
	ivinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;


	auto imageRes = m_boundedDevice->get_transfer_operation()->init_image_to_the_gpu_from_cpu_sleep(&info,&ivinfo,m_size,m_imageDescriptor->pixels);
	// Unload the m_imageDescriptor

	if (!imageRes.has_value())
	{
		return RESOURCE_INIT_CODE_UNKNOWN_EX;
	}
	
	m_gpuBuffer = imageRes.value();

	m_loader->unload(m_imageDescriptor);
	m_imageDescriptor = nullptr;


	// Now try to create sampler

	auto samplerRes = m_samplerCreator->create_sampler(m_boundedDevice);

	if(!samplerRes.has_value())
	{

		return RESOURCE_INIT_CODE_UNKNOWN_EX;
	}

	m_inUsageSampler = samplerRes.value();


	auto descriptorRes = m_descriptorCreator->create_descriptor_set_for_texture(m_gpuBuffer, m_inUsageSampler->get_vk_sampler());


	if (!descriptorRes.has_value())
	{
		return RESOURCE_INIT_CODE_UNKNOWN_EX;
	}

	m_descriptorSet = descriptorRes.value();

	return RESOURCE_INIT_CODE_OK;
}

void GTextureResource::unload_impl()
{
	if (m_imageDescriptor != nullptr && m_loader != nullptr)
	{
		m_loader->unload(m_imageDescriptor);
		m_imageDescriptor = nullptr;

	}
	if (m_gpuBuffer != nullptr)
	{
		m_gpuBuffer->unload();
		m_gpuBuffer = nullptr;
	}
	if (m_inUsageSampler != nullptr && m_samplerCreator != nullptr)
	{
		m_samplerCreator->destroy_sampler(m_inUsageSampler);
		m_inUsageSampler = nullptr;
	}
	if (m_descriptorSet != nullptr)
	{
		m_descriptorCreator->destroy_descriptor_set_dtor(m_descriptorSet);
		m_descriptorSet = nullptr;
	}
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

IGVulkanDescriptorSet* GTextureResource::get_descriptor_set() const
{
	return m_descriptorSet;
}

void GTextureResource::destroy_impl()
{
	m_creatorOwner->destroy_texture_resource(this);
}


IImageLoader* IGTextureResource::get_loader()
{
	return m_loader;
}
