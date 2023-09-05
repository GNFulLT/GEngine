#include "internal/engine/rendering/vulkan/gvulkan_offscreen_depth_viewport.h"
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
#include <array>

GVulkanOffScreenDepthViewport::GVulkanOffScreenDepthViewport(IGVulkanLogicalDevice* dev, IGVulkanDescriptorCreator* descriptorCreator)
{
	m_boundedDevice = dev;
	m_descriptorCreator = descriptorCreator;
	m_image = nullptr;
	m_depthImage = nullptr;
	m_descriptorSet = nullptr;
	m_sampler = nullptr;
	m_viewport.minDepth = 0;
	m_viewport.maxDepth = 1;
	m_viewport.x = 0;
	m_viewport.y = 0;
	m_viewport.width = 0;
	m_viewport.height = 0;
	m_scissor.offset = { 0,0 };

}

void* GVulkanOffScreenDepthViewport::get_vk_current_image_renderpass()
{
	return m_renderpass.get_vk_renderpass();
}

uint32_t GVulkanOffScreenDepthViewport::get_width() const
{
	return m_viewport.width;
}

uint32_t GVulkanOffScreenDepthViewport::get_height() const
{
	return m_viewport.height;
}

bool GVulkanOffScreenDepthViewport::init(int width, int height, int vkFormat)
{
	m_format = (VkFormat)vkFormat;
	m_depthFormat = VK_FORMAT_D16_UNORM;
	m_viewport.width = width;
	m_viewport.height = height;
	m_scissor.extent.width = width;
	m_scissor.extent.height = height;

	VkExtent3D extent = {
		.width = uint32_t(width),
		.height = uint32_t(height),
		.depth = 1
	};
	uint32_t renderingFamilyIndex = m_boundedDevice->get_render_queue()->get_queue_index();

	{
		// X TODO : HARD CODED
		


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
		auto res = m_boundedDevice->create_image(&inf, VMA_MEMORY_USAGE_CPU_COPY);

		if (!res.has_value())
		{
			// X TODO : LOGGER
			return false;
		}

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

	}
	
	//X Now create the depth image
	{
		VkImageCreateInfo inf = {};
		inf.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		inf.pNext = nullptr;
		inf.flags = 0;
		inf.imageType = VK_IMAGE_TYPE_2D;
		inf.format = m_depthFormat;
		inf.mipLevels = 1;
		inf.arrayLayers = 1;
		inf.samples = VK_SAMPLE_COUNT_1_BIT;
		inf.tiling = VK_IMAGE_TILING_OPTIMAL;
		inf.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		inf.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		inf.queueFamilyIndexCount = 1;
		inf.pQueueFamilyIndices = &renderingFamilyIndex;
		inf.extent = extent;
		inf.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		auto res = m_boundedDevice->create_image(&inf, VMA_MEMORY_USAGE_GPU_ONLY);
		if (!res.has_value())
		{
			// X TODO : LOGGER
			return false;
		}

		m_depthImage = res.value();
		

		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		range.layerCount = 1;
		range.baseMipLevel = 0;
		range.baseArrayLayer = 0;
		range.levelCount = 1;

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.format = m_depthFormat;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.flags = 0;
		viewInfo.subresourceRange = range;
	
		m_depthImage->create_image_view(&viewInfo);
	}


	std::vector<VkClearValue> clearValues;
	clearValues.push_back({ {0.f,1.f,0.f,1.f} });
	VkClearValue depthClear = {};
	depthClear.depthStencil.depth = 1.f;
	clearValues.push_back(depthClear);

	std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
	//X Render image attachment
	attchmentDescriptions[0].format = m_format;
	attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	//X Depth image attachment
	attchmentDescriptions[1].format = m_depthFormat;
	attchmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attchmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attchmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attchmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attchmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attchmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attchmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
	//X Attachment references
	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	//X Subpass descriptions for the attachments gave them the references
	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = &depthReference;


	//X Build the dependencies for layout transition
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
	renderPassInfo.pAttachments = attchmentDescriptions.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	m_renderpass.create(m_boundedDevice->get_vk_device(), m_image->get_vk_image_view(), extent.width, extent.height, clearValues, (VkFormat)vkFormat,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_SUBPASS_CONTENTS_INLINE, nullptr, 0,&renderPassInfo, m_depthImage->get_vk_image_view());

	if (m_renderpass.is_failed())
	{
		return false;
	}


	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;


	const VkPhysicalDeviceProperties* properties = m_boundedDevice->get_bounded_physical_device()->get_vk_properties();

	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 0.f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

	if (vkCreateSampler(m_boundedDevice->get_vk_device(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
		return false;
	}

	// Now create descriptors

	auto desRes = m_descriptorCreator->create_descriptor_set_for_texture(m_image, m_sampler);

	if (!desRes.has_value())
	{
		//X TODO : LOGGER
		return false;
	}

	m_descriptorSet = desRes.value();

	return true;
}

void GVulkanOffScreenDepthViewport::destroy(bool forResize)
{
	if (m_depthImage != nullptr)
	{
		m_depthImage->unload();
		delete m_depthImage;
		m_depthImage = nullptr;
	}
	if (m_image != nullptr)
	{
		m_image->unload();
		delete m_image;
		m_image = nullptr;
	}
	if (!m_renderpass.is_failed())
	{
		m_renderpass.destroy(m_boundedDevice->get_vk_device(), forResize);
	}
	if (m_sampler != nullptr)
	{
		vkDestroySampler(m_boundedDevice->get_vk_device(), m_sampler, nullptr);
		m_sampler = nullptr;
	}
	if (m_descriptorSet != nullptr)
	{
		m_descriptorSet->destroy_dtor();
		m_descriptorSet = nullptr;
	}
}

void GVulkanOffScreenDepthViewport::begin_draw_cmd(GVulkanCommandBuffer* cmd)
{
	cmd->begin();
	m_renderpass.begin(cmd->get_handle());
}

void GVulkanOffScreenDepthViewport::end_draw_cmd(GVulkanCommandBuffer* cmd)
{

	m_renderpass.end(cmd->get_handle());
	cmd->end();
}

IGVulkanRenderPass* GVulkanOffScreenDepthViewport::get_render_pass()
{
	return &m_renderpass;
}

bool GVulkanOffScreenDepthViewport::can_be_used_as_texture()
{
	return true;
}

IGVulkanDescriptorSet* GVulkanOffScreenDepthViewport::get_descriptor()
{
	return m_descriptorSet;
}

const VkViewport* GVulkanOffScreenDepthViewport::get_viewport_area() const noexcept
{
	return &m_viewport;
}

const VkRect2D* GVulkanOffScreenDepthViewport::get_scissor_area() const noexcept
{
	return &m_scissor;
}
