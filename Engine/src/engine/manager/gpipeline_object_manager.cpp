#include "volk.h"
#include "internal/engine/manager/gpipeline_object_manager.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/named/gvulkan_named_renderpass.h"
#include "internal/engine/rendering/vulkan/named/gvulkan_named_sampler.h"

GPipelineObjectManager::GPipelineObjectManager(IGVulkanLogicalDevice* logicalDevice,VkFormat swapchainFormat, uint32_t framesInFlight)
{
	m_framesInFlight = framesInFlight;
	m_swapchainFormat = swapchainFormat;
	m_logicalDevice = logicalDevice;
}

bool GPipelineObjectManager::init()
{
	return init_named_objects();
}

void GPipelineObjectManager::destroy()
{
	destroy_named_objects();
}

GSharedPtr<IGVulkanNamedRenderPass> GPipelineObjectManager::get_named_renderpass(const char* name)
{
	if (auto rp = m_namedRenderpassMap.find(std::string(name)); rp != m_namedRenderpassMap.end())
	{
		return rp->second;
	}
	return GSharedPtr<IGVulkanNamedRenderPass>();
}

GSharedPtr<IGVulkanNamedSampler> GPipelineObjectManager::get_named_sampler(const char* name)
{
	if (auto rp = m_namedSamplerMap.find(std::string(name)); rp != m_namedSamplerMap.end())
	{
		return rp->second;
	}
	return GSharedPtr<IGVulkanNamedSampler>();
}

bool GPipelineObjectManager::init_named_objects()
{
	return init_named_renderpass() && init_named_sampler();
}

void GPipelineObjectManager::destroy_named_objects()
{
	destroy_named_renderpass();
	destroy_named_sampler();
}

bool GPipelineObjectManager::init_named_renderpass()
{
	{
		std::array<VkAttachmentDescription, 2> attchmentDescriptions = {};
		//X Render image attachment
		attchmentDescriptions[0].format = m_swapchainFormat;
		attchmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attchmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attchmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attchmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attchmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attchmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attchmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		//X Depth image attachment
		attchmentDescriptions[1].format = VK_FORMAT_D16_UNORM;
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
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescriptions.size());
		renderPassInfo.pAttachments = attchmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpassDescription;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VkRenderPass renderPass;
		auto res = vkCreateRenderPass(m_logicalDevice->get_vk_device(), &renderPassInfo, nullptr, &renderPass);
		if (res != VK_SUCCESS)
		{
			return false;
		}
		
		auto ptr = GSharedPtr<IGVulkanNamedRenderPass>(new GNamedVulkanRenderPass(m_logicalDevice, "render_depth_pass", renderPass,m_swapchainFormat));
		m_namedRenderpassMap.emplace(std::string("render_depth_pass"),ptr);
		
	}
	{
		VkAttachmentDescription color_attachment = {};
		//the attachment will have the format needed by the swapchain
		color_attachment.format = m_swapchainFormat;
		//1 sample, we won't be doing MSAA
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		//X TODO:  No stencil for now
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		// Used as frame buffer
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkRenderPassCreateInfo create_inf = {};
		create_inf.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;


		VkAttachmentReference color_attachment_ref = {};
		//attachment number will index into the pAttachments array in the parent renderpass itself
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;

		//connect the color attachment to the info
		create_inf.attachmentCount = 1;
		create_inf.pAttachments = &color_attachment;
		//connect the subpass to the info
		create_inf.subpassCount = 1;
		create_inf.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		create_inf.dependencyCount = 1;
		create_inf.pDependencies = &dependency;
		
		VkRenderPass renderPass;
		auto res = vkCreateRenderPass(m_logicalDevice->get_vk_device(), &create_inf, nullptr, &renderPass);
		if (res != VK_SUCCESS)
		{
			return false;
		}

		auto ptr = GSharedPtr<IGVulkanNamedRenderPass>(new GNamedVulkanRenderPass(m_logicalDevice, "present_render_pass", renderPass,m_swapchainFormat));
		m_namedRenderpassMap.emplace(std::string("present_render_pass"), ptr);

	}
	return true;
}

void GPipelineObjectManager::destroy_named_renderpass()
{
	for (auto pair : m_namedRenderpassMap)
	{
		auto rp = &pair.second;
		if (rp->get_shared_ref_count() != 2)
		{
			//X TODO : LOG HERE
		}
		((GNamedVulkanRenderPass*)rp->get())->destroy();
	}
}

bool GPipelineObjectManager::init_named_sampler()
{
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 0;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = VK_LOD_CLAMP_NONE;


		VkSampler sampler = 0;
		auto res = vkCreateSampler(m_logicalDevice->get_vk_device(), &samplerInfo, nullptr, &sampler);
		if (VkResult::VK_SUCCESS != res)
		{
			return false;
		}
		auto ptr = GSharedPtr<IGVulkanNamedSampler>(new GVulkanNamedSampler(m_logicalDevice, "max_performant_sampler", sampler));
		m_namedSamplerMap.emplace(std::string("max_performant_sampler"), ptr);

	}
	return true;
}

void GPipelineObjectManager::destroy_named_sampler()
{
	for (auto pair : m_namedSamplerMap)
	{
		auto rp = &pair.second;
		if (rp->get_shared_ref_count() != 2)
		{
			//X TODO : LOG HERE
		}
		((GVulkanNamedSampler*)rp->get())->destroy();
	}
}
