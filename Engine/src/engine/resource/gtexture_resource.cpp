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

	if (!res.has_value())
	{
		auto err = res.error();
		switch (err) {
		case VULKAN_IMAGE_CREATION_ERROR_UNKNOWN:
			return RESOURCE_INIT_CODE::RESOURCE_INIT_CODE_UNKNOWN_EX;
		}
	}

	m_gpuBuffer = res.value();

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

	bool viewCreated = m_gpuBuffer->create_image_view(&ivinfo);

	if (!viewCreated)
		return RESOURCE_INIT_CODE_UNKNOWN_EX;

	// We are using cached memory
	// 	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	VkBufferUsageFlags flags = VkBufferUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	auto bufferRes = m_boundedDevice->create_buffer(m_size, flags,VMA_MEMORY_USAGE_CPU_ONLY);

	if (!bufferRes.has_value())
	{
		auto err = bufferRes.error();
		switch (err) {
		case VULKAN_BUFFER_CREATION_ERROR_UNKNOWN:
			return RESOURCE_INIT_CODE::RESOURCE_INIT_CODE_UNKNOWN_EX;
		}
	}

	auto imageBuffer = bufferRes.value();
	// Now we are creating buffer that will be copied to the image
	imageBuffer->copy_data_to_device_memory(m_imageDescriptor->pixels,m_size);

	// Ask a transfer queue from the device
	auto cmdRes = m_boundedDevice->get_wait_and_begin_transfer_cmd();

	if (!cmdRes.has_value())
	{
		auto err = cmdRes.error();
		// Timeout
		return RESOURCE_INIT_CODE::RESOURCE_INIT_CODE_UNKNOWN_EX;
	}

	auto transferHandle = cmdRes.value();
	auto cmd = transferHandle->get_command_buffer();
	
	
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		(unsigned int)m_imageDescriptor->width,
		(unsigned int)m_imageDescriptor->height,
		1
	};

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_gpuBuffer->get_vk_image();
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	auto  sourceStage = VK_PIPELINE_STAGE_HOST_BIT;
	auto destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

	vkCmdPipelineBarrier(
		cmd->get_handle(),
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);


	vkCmdCopyBufferToImage(
		cmd->get_handle(),
		imageBuffer->get_vk_buffer(),
		m_gpuBuffer->get_vk_image(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);


	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	vkCmdPipelineBarrier(
		cmd->get_handle(),
		sourceStage,
		destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	// Sleeps until command executing
	m_boundedDevice->finish_execute_and_wait_transfer_cmd(transferHandle);
	
	// Destroy the buffer
	imageBuffer->unload();

	//X TODO : GDNEWDA
	delete imageBuffer;

	// Unload the m_imageDescriptor

	m_loader->unload(m_imageDescriptor);
	m_imageDescriptor = nullptr;

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
