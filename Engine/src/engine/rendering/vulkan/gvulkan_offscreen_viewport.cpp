#include "internal/engine/rendering/vulkan/gvulkan_offscreen_viewport.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_pdevice.h"
#include <vma/vk_mem_alloc.h>
#include "engine/rendering/vulkan/ivulkan_queue.h"
#include "engine/rendering/vulkan/ivulkan_image.h"
#include <vector>
#include "engine/rendering/vulkan/ivulkan_descriptor_creator.h"
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include "internal/engine/manager/glogger_manager.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"

GVulkanOffScreenViewport::GVulkanOffScreenViewport(IGVulkanLogicalDevice* dev, IGVulkanDescriptorCreator* descriptorCreator)
{
	m_boundedDevice = dev;
	m_descriptorCreator = descriptorCreator;
	m_image = nullptr;
	m_descriptorSet = nullptr;
	m_sampler = nullptr;
}

GVulkanOffScreenViewport::~GVulkanOffScreenViewport()
{
	if (m_image != nullptr)
	{
		GLoggerManager::get_instance()->log_d("GVulkanOffScreenViewport", "Dont forget to release offscreen viewport !!!!");
	}
}

bool GVulkanOffScreenViewport::init(int width,int height,int vkFormat)
{
	VkExtent3D extent = {
		.width = uint32_t(width),
		.height = uint32_t(height),
		.depth = 1
	};

	uint32_t renderingFamilyIndex = m_boundedDevice->get_render_queue()->get_queue_index();

	VkImageCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	inf.pNext = nullptr;
	inf.flags = 0;
	inf.imageType = VK_IMAGE_TYPE_2D;
	inf.format = (VkFormat)vkFormat;
	inf.mipLevels = 1;
	inf.arrayLayers = 1;
	inf.samples = VK_SAMPLE_COUNT_1_BIT;
	inf.tiling = VK_IMAGE_TILING_OPTIMAL;
	inf.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	inf.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	inf.queueFamilyIndexCount = 1;
	inf.pQueueFamilyIndices = &renderingFamilyIndex;
	inf.extent = extent;
	inf.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// VMA_MEMORY_USAGE_CPU_COPY intented to copy the image from gpu to use as a texture the last image
	auto res = m_boundedDevice->create_image(&inf,VMA_MEMORY_USAGE_CPU_COPY);

	if (!res.has_value())
	{
		// X TODO : LOGGER
		return false;
	}

	// We created the image that we will render now create a imageview for it

	m_image = res.value();

	VkImageSubresourceRange range = {};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.layerCount = 1;
	range.baseMipLevel = 0;
	range.baseArrayLayer = 0;
	range.levelCount = 1;
	
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.pNext = nullptr;
	viewInfo.format = (VkFormat)vkFormat;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.flags = 0;
	viewInfo.subresourceRange = range;
	viewInfo.image = m_image->get_vk_image();
	
	m_image->create_image_view(&viewInfo);

	
	

	// We created the renderpass

	std::vector<VkClearValue> clearValues;
	clearValues.push_back({ {0.f,1.f,0.f,1.f} });

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0; // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


	m_renderpass.create((VkDevice)m_boundedDevice->get_vk_device(), m_image->get_vk_image_view(), extent.width, extent.height, clearValues, (VkFormat)vkFormat,
	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL , VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,VK_SUBPASS_CONTENTS_INLINE,&dependency,1);

	if (m_renderpass.is_failed())
	{
		return false;
	}


	// For using as texture we must create sampler

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;	
	samplerInfo.anisotropyEnable = VK_TRUE;


	const VkPhysicalDeviceProperties* properties = m_boundedDevice->get_bounded_physical_device()->get_vk_properties();

	samplerInfo.maxAnisotropy = properties->limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(m_boundedDevice->get_vk_device(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
		return false;
	}

	// Now create descriptors

	auto desRes = m_descriptorCreator->create_descriptor_set_for_texture(m_image,m_sampler);
	
	if(!desRes.has_value())
	{
		//X TODO : LOGGER
		return false;
	}

	m_descriptorSet = desRes.value();

	return true;
}

void GVulkanOffScreenViewport::destroy()
{
	if (m_image != nullptr)
	{
		m_image->unload();
		delete m_image;
		m_image = nullptr;
	}
	if (!m_renderpass.is_failed())
	{
		m_renderpass.destroy(m_boundedDevice->get_vk_device());
	}
	if (m_sampler != nullptr)
	{
		vkDestroySampler(m_boundedDevice->get_vk_device(),m_sampler,nullptr);
		m_sampler = nullptr;
	}
	if (m_descriptorSet != nullptr)
	{
		m_descriptorSet->destroy_dtor();
		m_descriptorSet = nullptr;
	}
}

void* GVulkanOffScreenViewport::get_vk_current_image_renderpass()
{
	return nullptr;
}

uint32_t GVulkanOffScreenViewport::get_width() const
{
	return 0;
}

uint32_t GVulkanOffScreenViewport::get_height() const
{
	return 0;
}

void GVulkanOffScreenViewport::begin_draw_cmd(GVulkanCommandBuffer* cmd)
{
	cmd->begin();
	m_renderpass.begin(cmd->get_handle());
}

void GVulkanOffScreenViewport::end_draw_cmd(GVulkanCommandBuffer* cmd)
{
	m_renderpass.end(cmd->get_handle());
	cmd->end();
}

bool GVulkanOffScreenViewport::can_be_used_as_texture()
{
	return true;
}

IGVulkanDescriptorSet* GVulkanOffScreenViewport::get_descriptor()
{
	return m_descriptorSet;
}

IGVulkanRenderPass* GVulkanOffScreenViewport::get_render_pass()
{
	return &m_renderpass;
}
