#include "internal/engine/rendering/vulkan/named/gvulkan_named_deferred_viewport.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_queue.h"
#include "vma/vk_mem_alloc.h"
#include "internal/engine/rendering/vulkan/named/gvulkan_named_renderpass.h"

#define POSITION_TARGET_COLOR_FORMAT VK_FORMAT_R32G32B32A32_SFLOAT
#define NORMAL_TARGET_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define ALBEDO_TARGET_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define PBR_TARGET_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM
#define DEPTH_TARGET_COLOR_FORMAT VK_FORMAT_D32_SFLOAT
#define COMPOSITION_TARGET_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM

GVulkanNamedDeferredViewport::GVulkanNamedDeferredViewport(IGVulkanLogicalDevice* dev, std::string_view name)
{
	m_secondFrameBuffer = nullptr;
	m_firstPass = nullptr;
	m_secondPass = nullptr;
	m_boundedDevice = dev;
	m_name = name;
	m_albedoAttachment = nullptr;
	m_normalAttachment = nullptr;
	m_positionAttachment = nullptr;
	m_viewport.x = 0;
	m_viewport.y = 0;
	m_viewport.minDepth = 0;
	m_viewport.maxDepth = 1.F;
	m_scissor.offset = { 0,0 };
	
	m_attachments.emplace("position_attachment",nullptr);
	m_attachments.emplace("normal_attachment", nullptr);
	m_attachments.emplace("albedo_attachment", nullptr);
	m_attachments.emplace("depth_attachment", nullptr);
	m_attachments.emplace("composition_attachment", nullptr);

	m_renderPassBeginInfo = {};

	m_frameBuffer = nullptr;
	m_renderPass = nullptr;

	clearValues[0].color = { { 0.0f, 1.0f, 0.0f, 1.0f } };
	clearValues[1].color = { { 1.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[2].color = { { 0.0f, 0.0f, 1.0f, 1.0f } };
	clearValues[3].depthStencil = { 1.0f, 0 };

	secondClearValues[0] = { { 1.0f, 0.0f, 0.0f, 1.0f } };
	secondClearValues[1].depthStencil = { 1.0f, 0 };

}

GVulkanNamedDeferredViewport::~GVulkanNamedDeferredViewport()
{
	vkDestroyRenderPass(m_boundedDevice->get_vk_device(), m_renderPass, nullptr);
}

const char* GVulkanNamedDeferredViewport::get_name() const noexcept
{
	return m_name.data();
}

const VkViewport* GVulkanNamedDeferredViewport::get_viewport_area() const noexcept
{
	return &m_viewport;
}

const VkRect2D* GVulkanNamedDeferredViewport::get_scissor_area() const noexcept
{
	return &m_scissor;
}

void GVulkanNamedDeferredViewport::begin_draw_cmd(GVulkanCommandBuffer* cmd)
{
	vkCmdBeginRenderPass(cmd->get_handle(), &m_renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void GVulkanNamedDeferredViewport::end_draw_cmd(GVulkanCommandBuffer* cmd)
{
	vkCmdEndRenderPass(cmd->get_handle());
}

bool GVulkanNamedDeferredViewport::is_sampled() const noexcept
{
	return true;
}

IVulkanImage* GVulkanNamedDeferredViewport::get_named_attachment(const std::string& name) const noexcept
{
	if (auto attachment = m_attachments.find(name); attachment != m_attachments.end())
	{
		return attachment->second;
	}
	return nullptr;
}

bool GVulkanNamedDeferredViewport::init(uint32_t width, uint32_t height)
{
	//X Create position texture
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
	//X Position Attachment
	{
		VkImageCreateInfo inf = {};
		inf.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		inf.pNext = nullptr;
		inf.flags = 0;
		inf.imageType = VK_IMAGE_TYPE_2D;
		inf.format = POSITION_TARGET_COLOR_FORMAT;
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
		viewInfo.format = POSITION_TARGET_COLOR_FORMAT;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.flags = 0;
		viewInfo.subresourceRange = range;
		viewInfo.image = m_positionAttachment->get_vk_image();

		m_positionAttachment->create_image_view(&viewInfo);

		//X Now create for normal

		inf.format = NORMAL_TARGET_COLOR_FORMAT;
		res = m_boundedDevice->create_image(&inf, VMA_MEMORY_USAGE_GPU_ONLY);

		if (!res.has_value())
		{
			// X TODO : LOGGER
			return false;
		}
		m_normalAttachment = res.value();

		viewInfo.format = NORMAL_TARGET_COLOR_FORMAT;
		m_normalAttachment->create_image_view(&viewInfo);

		inf.format = ALBEDO_TARGET_COLOR_FORMAT;
		res = m_boundedDevice->create_image(&inf, VMA_MEMORY_USAGE_GPU_ONLY);

		if (!res.has_value())
		{
			// X TODO : LOGGER
			return false;
		}
		m_albedoAttachment = res.value();
		viewInfo.format = ALBEDO_TARGET_COLOR_FORMAT;
		m_albedoAttachment->create_image_view(&viewInfo);


		inf.format = COMPOSITION_TARGET_COLOR_FORMAT;
		viewInfo.format = COMPOSITION_TARGET_COLOR_FORMAT;
		res = m_boundedDevice->create_image(&inf, VMA_MEMORY_USAGE_GPU_ONLY);
		if (!res.has_value())
		{
			// X TODO : LOGGER
			return false;
		}
		m_composedAttachment = res.value();
		m_composedAttachment->create_image_view(&viewInfo);
		inf.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		inf.format = DEPTH_TARGET_COLOR_FORMAT;
		viewInfo.format = DEPTH_TARGET_COLOR_FORMAT;
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
	if(m_renderPass == nullptr)
	{
		//X Create the renderpass 
		//X First pass
		{
			std::array<VkAttachmentDescription, 4> attachmentDescs = {};
			for (uint32_t i = 0; i < 4; ++i)
			{
				attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				//X Last one will be for depth attachment
				if (i == 3)
				{
					attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
				else
				{
					attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
			}
			attachmentDescs[0].format = POSITION_TARGET_COLOR_FORMAT;
			attachmentDescs[1].format = NORMAL_TARGET_COLOR_FORMAT;
			attachmentDescs[2].format = ALBEDO_TARGET_COLOR_FORMAT;
			attachmentDescs[3].format = DEPTH_TARGET_COLOR_FORMAT;

			std::vector<VkAttachmentReference> colorReferences;
			colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
			colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

			VkAttachmentReference depthReference = {};
			depthReference.attachment = 3;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.pColorAttachments = colorReferences.data();
			subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
			subpass.pDepthStencilAttachment = &depthReference;

			// Use subpass dependencies for attachment layout transitions
			std::array<VkSubpassDependency, 2> dependencies;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = 0;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = 0;

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.pAttachments = attachmentDescs.data();
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 2;
			renderPassInfo.pDependencies = dependencies.data();

			vkCreateRenderPass(m_boundedDevice->get_vk_device(), &renderPassInfo, nullptr, &m_renderPass);

			m_firstPass = new GNamedVulkanRenderPass(m_boundedDevice, FIRST_RENDER_PASS.data(), m_renderPass, POSITION_TARGET_COLOR_FORMAT);
		}


		//X Clean composition pass
		{
			std::array<VkAttachmentDescription, 2> attachmentDescs = {};
			for (uint32_t i = 0; i < 2; ++i)
			{
				attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				//X Last one will be for depth attachment
				if (i == 1)
				{
					attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				}
				else
				{
					attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
			}
			attachmentDescs[0].format = COMPOSITION_TARGET_COLOR_FORMAT;
			attachmentDescs[1].format = DEPTH_TARGET_COLOR_FORMAT;


			std::vector<VkAttachmentReference> colorReferences;
			colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

			VkAttachmentReference depthReference = {};
			depthReference.attachment = 1;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			
			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.pColorAttachments = colorReferences.data();
			subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
			subpass.pDepthStencilAttachment = &depthReference;

			// Use subpass dependencies for attachment layout transitions
			std::array<VkSubpassDependency, 2> dependencies;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = 0;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = 0;

			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.pAttachments = attachmentDescs.data();
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = 2;
			renderPassInfo.pDependencies = dependencies.data();
			VkRenderPass secondPass;
			vkCreateRenderPass(m_boundedDevice->get_vk_device(), &renderPassInfo, nullptr, &secondPass);
			m_secondPass = new GNamedVulkanRenderPass(m_boundedDevice, SECOND_RENDER_PASS.data(), secondPass, COMPOSITION_TARGET_COLOR_FORMAT);

			m_namedRenderpasses.emplace(FIRST_RENDER_PASS.data(), m_firstPass);
			m_namedRenderpasses.emplace(SECOND_RENDER_PASS.data(), m_secondPass);

		}
	}

	//X Create Frame Buffer
	{
		{
			std::array<VkImageView, 4> attachments;
			attachments[0] = m_positionAttachment->get_vk_image_view();
			attachments[1] = m_normalAttachment->get_vk_image_view();
			attachments[2] = m_albedoAttachment->get_vk_image_view();
			attachments[3] = m_depthAttachment->get_vk_image_view();

			VkFramebufferCreateInfo fbufCreateInfo = {};
			fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbufCreateInfo.pNext = NULL;
			fbufCreateInfo.renderPass = m_renderPass;
			fbufCreateInfo.pAttachments = attachments.data();
			fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			fbufCreateInfo.width = width;
			fbufCreateInfo.height = height;
			fbufCreateInfo.layers = 1;
			vkCreateFramebuffer(m_boundedDevice->get_vk_device(), &fbufCreateInfo, nullptr, &m_frameBuffer);
		}
		{
			std::array<VkImageView, 2> attachments;
			attachments[0] = m_composedAttachment->get_vk_image_view();
			attachments[1] = m_depthAttachment->get_vk_image_view();
			VkFramebufferCreateInfo fbufCreateInfo = {};
			fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbufCreateInfo.pNext = NULL;
			fbufCreateInfo.renderPass = m_secondPass->get_vk_renderpass();
			fbufCreateInfo.pAttachments = attachments.data();
			fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			fbufCreateInfo.width = width;
			fbufCreateInfo.height = height;
			fbufCreateInfo.layers = 1;
			vkCreateFramebuffer(m_boundedDevice->get_vk_device(), &fbufCreateInfo, nullptr, &m_secondFrameBuffer);
		}

		// Create sampler to sample from the color attachments
		VkSamplerCreateInfo sampler = {};
		sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		sampler.magFilter = VK_FILTER_LINEAR;
		sampler.minFilter = VK_FILTER_LINEAR;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		sampler.addressModeV = sampler.addressModeU;
		sampler.addressModeW = sampler.addressModeU;
		sampler.mipLodBias = 0.0f;
		sampler.maxAnisotropy = 0.0f;
		sampler.minLod = 0.0f;
		sampler.maxLod = 1.0f;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		vkCreateSampler(m_boundedDevice->get_vk_device(), &sampler, nullptr, &m_sampler);
	}
	m_attachments["position_attachment"] = m_positionAttachment;
	m_attachments["normal_attachment"] = m_normalAttachment;
	m_attachments["albedo_attachment"] = m_albedoAttachment;
	m_attachments["depth_attachment"] = m_depthAttachment;
	m_attachments["composition_attachment"] = m_composedAttachment;


	m_renderPassBeginInfo = {};
	m_renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	m_renderPassBeginInfo.renderPass = m_renderPass;
	m_renderPassBeginInfo.framebuffer = m_frameBuffer;
	m_renderPassBeginInfo.renderArea.extent.width =width;
	m_renderPassBeginInfo.renderArea.extent.height = height;
	m_renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	m_renderPassBeginInfo.pClearValues = clearValues.data();


	m_secondPassBeginInfo = {};
	m_secondPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	m_secondPassBeginInfo.renderPass = m_secondPass->get_vk_renderpass();
	m_secondPassBeginInfo.framebuffer = m_secondFrameBuffer;
	m_secondPassBeginInfo.renderArea.extent.width = width;
	m_secondPassBeginInfo.renderArea.extent.height = height;
	m_secondPassBeginInfo.clearValueCount = static_cast<uint32_t>(secondClearValues.size());
	m_secondPassBeginInfo.pClearValues = secondClearValues.data();
	return true;

}

void GVulkanNamedDeferredViewport::destroy()
{
	if (m_frameBuffer != nullptr)
		vkDestroyFramebuffer(m_boundedDevice->get_vk_device(), m_frameBuffer, nullptr);
	if (m_depthAttachment != nullptr)
	{
		m_depthAttachment->unload();
		delete m_depthAttachment;
		m_depthAttachment = nullptr;
	}
	if (m_albedoAttachment != nullptr)
	{
		m_albedoAttachment->unload();
		delete m_albedoAttachment;
		m_albedoAttachment = nullptr;
	}
	if (m_normalAttachment != nullptr)
	{
		m_normalAttachment->unload();
		delete m_normalAttachment;
		m_normalAttachment = nullptr;
	}
	if (m_positionAttachment != nullptr)
	{
		m_positionAttachment->unload();
		delete m_positionAttachment;
		m_positionAttachment = nullptr;
	}
	if (m_composedAttachment != nullptr)
	{
		m_composedAttachment->unload();
		delete m_composedAttachment;
		m_composedAttachment = nullptr;
	}
	m_frameBuffer = nullptr;

	m_attachments["position_attachment"] = nullptr;
	m_attachments["normal_attachment"] = nullptr;
	m_attachments["albedo_attachment"] = nullptr;
	m_attachments["depth_attachment"] = nullptr;
}

bool GVulkanNamedDeferredViewport::resize(uint32_t width, uint32_t height)
{
	destroy();
	return init(width,height);
}

VkSampler_T* GVulkanNamedDeferredViewport::get_sampler_for_named_attachment(const std::string& name) const noexcept
{
	return m_sampler;
}

IGVulkanRenderPass* GVulkanNamedDeferredViewport::get_dedicated_renderpass() const noexcept
{
	return m_firstPass;
}

IGVulkanNamedRenderPass* GVulkanNamedDeferredViewport::get_named_renderpass(const char* name) const noexcept
{
	if (auto pass = m_namedRenderpasses.find(std::string(name)); pass != m_namedRenderpasses.end())
	{
		return pass->second;
	}
	return nullptr;
}

bool GVulkanNamedDeferredViewport::begin_draw_cmd_to_named_pass(GVulkanCommandBuffer* cmd, const char* name)
{
	if (strcmp(name, FIRST_RENDER_PASS.data()) == 0)
	{
		begin_draw_cmd(cmd);
		return true;
	}
	else if (strcmp(name, SECOND_RENDER_PASS.data()) == 0)
	{
		vkCmdBeginRenderPass(cmd->get_handle(), &m_secondPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		return true;
	}
	return false;
}

