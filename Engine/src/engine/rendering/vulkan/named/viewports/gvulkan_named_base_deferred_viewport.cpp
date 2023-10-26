#include "internal/engine/rendering/vulkan/named/viewports/gvulkan_named_base_deferred_viewport.h"
#include "engine/rendering/vulkan/ivulkan_queue.h"
#include "vk_mem_alloc.h"

#include <array>

GVulkanNamedBaseDeferredViewport::GVulkanNamedBaseDeferredViewport(IGVulkanLogicalDevice* dev, IGVulkanNamedRenderPass* deferredPass, IGVulkanNamedRenderPass* compositionPass, VkFormat positionFormat,
	VkFormat albedoFormat, VkFormat emissionFormat, VkFormat pbrFormmat, VkFormat compositionFormat)
{
	m_compositionPass = compositionPass;
	m_compositionFormat = compositionFormat;
	m_boundedDevice = dev;
	m_deferredPass = deferredPass;
	m_positionFormat = positionFormat;
	m_albedoFormat = albedoFormat;
	m_emissionFormat = emissionFormat;
	m_pbrFormat = pbrFormmat;
	m_depthFormat = VK_FORMAT_D32_SFLOAT;

	m_name = "BaseDeferredViewport";

	m_viewport.x = 0;
	m_viewport.y = 0;
	m_viewport.maxDepth = 1;
	m_viewport.minDepth = 0;
	m_viewport.width = 0;
	m_viewport.height = 0;

	m_scissor.offset = { 0,0 };


	clearValues[0].color = { { 0.0f, 1.0f, 0.0f, 1.0f } };
	clearValues[1].color = { { 1.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[2].color = { { 0.0f, 0.0f, 1.0f, 1.0f } };
	clearValues[3].color = { { 1.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[4].depthStencil = { 1.0f, 0 };

	secondClearValues[0] = { { 0.0f, 1.0f, 0.0f, 1.0f } };
	secondClearValues[1].depthStencil = { 1.0f, 0 };
}

const char* GVulkanNamedBaseDeferredViewport::get_name() const noexcept
{
	return m_name.c_str();
}

const VkViewport* GVulkanNamedBaseDeferredViewport::get_viewport_area() const noexcept
{
	return &m_viewport;
}

const VkRect2D* GVulkanNamedBaseDeferredViewport::get_scissor_area() const noexcept
{
	return &m_scissor;
}

void GVulkanNamedBaseDeferredViewport::begin_draw_cmd(GVulkanCommandBuffer* cmd)
{
	vkCmdBeginRenderPass(cmd->get_handle(), &m_deferredBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void GVulkanNamedBaseDeferredViewport::end_draw_cmd(GVulkanCommandBuffer* cmd)
{
	vkCmdEndRenderPass(cmd->get_handle());
}

bool GVulkanNamedBaseDeferredViewport::init(uint32_t width, uint32_t height)
{
	//X Create attachments
	m_viewport.width = width;
	
	//x Flip Height
	int intHeight = height;

	m_viewport.y = height;

	m_viewport.height = -intHeight;

	m_scissor.extent.width = width;
	m_scissor.extent.height = height;

	VkExtent3D extent = {
		.width = uint32_t(width),
		.height = uint32_t(height),
		.depth = 1
	};
	uint32_t renderingFamilyIndex = m_boundedDevice->get_render_queue()->get_queue_index();
		//X Position Attachment
	{
		VkImageCreateInfo inf = {};
		inf.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		inf.pNext = nullptr;
		inf.flags = 0;
		inf.imageType = VK_IMAGE_TYPE_2D;
		inf.format = m_positionFormat;
		inf.mipLevels = 1;
		inf.arrayLayers = 1;
		inf.samples = VK_SAMPLE_COUNT_1_BIT;
		inf.tiling = VK_IMAGE_TILING_OPTIMAL;
		inf.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
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

		m_positionAttachment = res.value();

		VkImageSubresourceRange range = {};
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.layerCount = 1;
		range.baseMipLevel = 0;
		range.baseArrayLayer = 0;
		range.levelCount = 1;

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.pNext = nullptr;
		viewInfo.format = m_positionFormat;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.flags = 0;
		viewInfo.subresourceRange = range;
		viewInfo.image = m_positionAttachment->get_vk_image();
		m_positionAttachment->create_image_view(&viewInfo);

		//X Now create for albedo

		inf.format = m_albedoFormat;
		res = m_boundedDevice->create_image(&inf, VMA_MEMORY_USAGE_GPU_ONLY);

		if (!res.has_value())
		{
			// X TODO : LOGGER
			return false;
		}
		m_albedoAttachment = res.value();

		viewInfo.format = m_albedoFormat;
		m_albedoAttachment->create_image_view(&viewInfo);
		inf.format = m_emissionFormat;
		res = m_boundedDevice->create_image(&inf, VMA_MEMORY_USAGE_GPU_ONLY);

		if (!res.has_value())
		{
			// X TODO : LOGGER
			return false;
		}
		m_emissionAttachment = res.value();
		viewInfo.format = m_emissionFormat;
		m_emissionAttachment->create_image_view(&viewInfo);

		//X Create PBR attachment
		inf.format = m_pbrFormat;
		viewInfo.format = m_pbrFormat;
		res = m_boundedDevice->create_image(&inf, VMA_MEMORY_USAGE_GPU_ONLY);
		if (!res.has_value())
		{
			// X TODO : LOGGER
			return false;
		}
		m_pbrAttachment = res.value();
		m_pbrAttachment->create_image_view(&viewInfo);

		inf.format = m_compositionFormat;
		viewInfo.format = m_compositionFormat;

		res = m_boundedDevice->create_image(&inf, VMA_MEMORY_USAGE_GPU_ONLY);
		if (!res.has_value())
		{
			// X TODO : LOGGER
			return false;
		}
		m_compositionAttachment = res.value();
		m_compositionAttachment->create_image_view(&viewInfo);

		//X Create depth attachment
		inf.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		inf.format = m_depthFormat;
		viewInfo.format = m_depthFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		res = m_boundedDevice->create_image(&inf, VMA_MEMORY_USAGE_GPU_ONLY);

		if (!res.has_value())
		{
			// X TODO : LOGGER
			return false;
		}
		m_depthAttachment = res.value();
		m_depthAttachment->create_image_view(&viewInfo);
	}

	//X Create Framebuffer
	{
		//X Deferred
		{
			std::array<VkImageView, 5> attachments;
			attachments[0] = m_positionAttachment->get_vk_image_view();
			attachments[1] = m_albedoAttachment->get_vk_image_view();
			attachments[2] = m_emissionAttachment->get_vk_image_view();
			attachments[3] = m_pbrAttachment->get_vk_image_view();
			attachments[4] = m_depthAttachment->get_vk_image_view();

			VkFramebufferCreateInfo fbufCreateInfo = {};
			fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbufCreateInfo.pNext = NULL;
			fbufCreateInfo.renderPass = m_deferredPass->get_vk_renderpass();
			fbufCreateInfo.pAttachments = attachments.data();
			fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			fbufCreateInfo.width = width;
			fbufCreateInfo.height = height;
			fbufCreateInfo.layers = 1;
			vkCreateFramebuffer(m_boundedDevice->get_vk_device(), &fbufCreateInfo, nullptr, &m_deferredFrameBuffer);
		}
		//X Composition
		{
			std::array<VkImageView, 2> attachments;
			attachments[0] = m_compositionAttachment->get_vk_image_view();
			attachments[1] = m_depthAttachment->get_vk_image_view();

			VkFramebufferCreateInfo fbufCreateInfo = {};
			fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbufCreateInfo.pNext = NULL;
			fbufCreateInfo.renderPass = m_compositionPass->get_vk_renderpass();
			fbufCreateInfo.pAttachments = attachments.data();
			fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			fbufCreateInfo.width = width;
			fbufCreateInfo.height = height;
			fbufCreateInfo.layers = 1;
			vkCreateFramebuffer(m_boundedDevice->get_vk_device(), &fbufCreateInfo, nullptr, &m_compositionFramebuffer);
		}
	}
	//X Create default sampler
	{
		// Create sampler to sample from the color attachments
		VkSamplerCreateInfo sampler = {};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.magFilter = VK_FILTER_LINEAR;
		sampler.minFilter = VK_FILTER_LINEAR;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		sampler.addressModeV = sampler.addressModeU;
		sampler.addressModeW = sampler.addressModeU;
		sampler.mipLodBias = 0.0f;
		sampler.maxAnisotropy = 0.0f;
		sampler.minLod = 0.0f;
		sampler.maxLod = 1.0f;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		vkCreateSampler(m_boundedDevice->get_vk_device(), &sampler, nullptr, &m_sampler);
	}
	//X Set begin info
	{
		m_deferredBeginInfo = {};
		m_deferredBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		m_deferredBeginInfo.renderPass = m_deferredPass->get_vk_renderpass();
		m_deferredBeginInfo.framebuffer = m_deferredFrameBuffer;
		m_deferredBeginInfo.renderArea.extent.width = width;
		m_deferredBeginInfo.renderArea.extent.height = height;
		m_deferredBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		m_deferredBeginInfo.pClearValues = clearValues.data();


		m_compositionBeginInfo = {};
		m_compositionBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		m_compositionBeginInfo.renderPass = m_compositionPass->get_vk_renderpass();
		m_compositionBeginInfo.framebuffer = m_compositionFramebuffer;
		m_compositionBeginInfo.renderArea.extent.width = width;
		m_compositionBeginInfo.renderArea.extent.height = height;
		m_compositionBeginInfo.clearValueCount = static_cast<uint32_t>(secondClearValues.size());
		m_compositionBeginInfo.pClearValues = secondClearValues.data();
		return true;

	}
}

void GVulkanNamedBaseDeferredViewport::destroy()
{
	if (m_deferredFrameBuffer != nullptr)
	{
		vkDestroyFramebuffer(m_boundedDevice->get_vk_device(), m_deferredFrameBuffer, nullptr);
		m_deferredFrameBuffer = nullptr;
	}
	if (m_compositionFramebuffer != nullptr)
	{
		vkDestroyFramebuffer(m_boundedDevice->get_vk_device(), m_compositionFramebuffer, nullptr);
		m_compositionFramebuffer = nullptr;
	}
	if (m_positionAttachment != nullptr)
	{
		m_positionAttachment->unload();
		delete m_positionAttachment;
		m_positionAttachment = nullptr;
	}
	if (m_albedoAttachment != nullptr)
	{
		m_albedoAttachment->unload();
		delete m_albedoAttachment;
		m_albedoAttachment = nullptr;
	}
	if (m_emissionAttachment != nullptr)
	{
		m_emissionAttachment->unload();
		delete m_emissionAttachment;
		m_emissionAttachment = nullptr;
	}
	if (m_pbrAttachment != nullptr)
	{
		m_pbrAttachment->unload();
		delete m_pbrAttachment;
		m_pbrAttachment = nullptr;
	}
	if (m_compositionAttachment != nullptr)
	{
		m_compositionAttachment->unload();
		delete m_compositionAttachment;
		m_compositionAttachment = nullptr;
	}
	if (m_depthAttachment != nullptr)
	{
		m_depthAttachment->unload();
		delete m_depthAttachment;
		m_depthAttachment = nullptr;
	}
	if (m_sampler != nullptr)
	{
		vkDestroySampler(m_boundedDevice->get_vk_device(),m_sampler,nullptr);
		m_sampler = nullptr;
	}
}

bool GVulkanNamedBaseDeferredViewport::resize(uint32_t width, uint32_t height)
{
	destroy();
	return init(width,height);
}

IVulkanImage* GVulkanNamedBaseDeferredViewport::get_albedo_attachment() const noexcept
{
	return m_albedoAttachment;
}

IVulkanImage* GVulkanNamedBaseDeferredViewport::get_emission_attachment() const noexcept
{
	return m_emissionAttachment;
}

IVulkanImage* GVulkanNamedBaseDeferredViewport::get_position_attachment() const noexcept
{
	return m_positionAttachment;
}

IVulkanImage* GVulkanNamedBaseDeferredViewport::get_pbr_attachment() const noexcept
{
	return m_pbrAttachment;
}

IGVulkanNamedRenderPass* GVulkanNamedBaseDeferredViewport::get_deferred_renderpass() const noexcept
{
	return m_deferredPass;;
}

bool GVulkanNamedBaseDeferredViewport::is_sampled() const noexcept
{
	return true;
}

VkSampler_T* GVulkanNamedBaseDeferredViewport::get_sampler_for_named_attachment(const std::string& name) const noexcept
{
	return m_sampler;
}

IVulkanImage* GVulkanNamedBaseDeferredViewport::get_composition_attachment() const noexcept
{
	return m_compositionAttachment;
}

IGVulkanNamedRenderPass* GVulkanNamedBaseDeferredViewport::get_composition_renderpass() const noexcept
{
	return m_compositionPass;
}

void GVulkanNamedBaseDeferredViewport::begin_composition_draw_cmd(GVulkanCommandBuffer* cmd)
{
	vkCmdBeginRenderPass(cmd->get_handle(), &m_compositionBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}
