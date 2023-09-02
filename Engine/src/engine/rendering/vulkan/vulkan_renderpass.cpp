#include "internal/engine/rendering/vulkan/vulkan_renderpass.h"

GVulkanRenderpass::GVulkanRenderpass()
{	
	_info = {};
	_isFailed = true;
	_subpassContents = VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE;
}

void GVulkanRenderpass::create(VkDevice dev, const std::vector<VkImageView>& views, std::vector<C_GVec2>& sizes, const std::vector<VkClearValue>& clearValues, VkFormat format, VkImageLayout finalLayout, VkImageLayout attachmentReferenceLayout, VkSubpassContents subpassContents)
{
	VkRenderPass handle = nullptr;

	if (_info.size() > 0 && _info[0].renderPass != nullptr)
	{
		handle = _info[0].renderPass;
	}

	_info = {};

	_subpassContents = subpassContents;

	_clearValues = clearValues;
	VkAttachmentDescription color_attachment = {};
	//the attachment will have the format needed by the swapchain
	color_attachment.format = format;
	//1 sample, we won't be doing MSAA
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	//X TODO:  No stencil for now
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Used as frame buffer
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	color_attachment.finalLayout = finalLayout;

	VkRenderPassCreateInfo create_inf = {};
	create_inf.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;


	VkAttachmentReference color_attachment_ref = {};
	//attachment number will index into the pAttachments array in the parent renderpass itself
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = attachmentReferenceLayout;

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
	
	if (handle != nullptr)
	{
		int a = 5;
		_isFailed = false;
	}
	else
	{
		if (VK_SUCCESS != vkCreateRenderPass(dev, &create_inf, nullptr, &handle))
		{
			_isFailed = true;
		}
		else
		{
			_isFailed = false;
		}
	}

	if (!_isFailed)
	{
		_info.resize(views.size());
		for (int i = 0; i < views.size(); i++)
		{
			VkFramebufferCreateInfo fb_info = {};
			fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fb_info.pNext = nullptr;

			fb_info.renderPass = handle;
			fb_info.attachmentCount = 1;
			fb_info.width = sizes[i].x;
			fb_info.height = sizes[i].y;
			fb_info.layers = 1;
			fb_info.pAttachments = &views[i];
			VkFramebuffer frameBuffer;
			if (vkCreateFramebuffer(dev, &fb_info, nullptr, &frameBuffer) != VK_SUCCESS)
			{
				_isFailed = true;
				break;
			}
			else
			{
				_isFailed = false;
				_info[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				_info[i].renderPass = handle;
				_info[i].framebuffer = frameBuffer;
				_info[i].renderArea.extent.width = sizes[i].x;
				_info[i].renderArea.extent.height = sizes[i].y;
				_info[i].clearValueCount = (uint32_t)_clearValues.size();
				_info[i].pClearValues = _clearValues.data();
			}
		}
	}
}


void GVulkanRenderpass::create(VkDevice dev, VkImageView imageView, uint32_t width, uint32_t height, const std::vector<VkClearValue>& clearValues, VkFormat format, VkImageLayout finalLayout, VkImageLayout attachmentReferenceLayout, VkSubpassContents subpassContents, VkSubpassDependency* dependency, int dependencyCount)
{
	VkRenderPass handle = nullptr;

	if (_info.size() > 0 && _info[0].renderPass != nullptr)
	{
		handle = _info[0].renderPass;
	}
	_info = {};

	_subpassContents = subpassContents;

	_clearValues = clearValues;
	VkAttachmentDescription color_attachment = {};
	//the attachment will have the format needed by the swapchain
	color_attachment.format = format;
	//1 sample, we won't be doing MSAA
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	//X TODO:  No stencil for now
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Used as frame buffer
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	color_attachment.finalLayout = finalLayout;

	VkRenderPassCreateInfo create_inf = {};
	create_inf.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;


	VkAttachmentReference color_attachment_ref = {};
	//attachment number will index into the pAttachments array in the parent renderpass itself
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = attachmentReferenceLayout;

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

	if (dependency != nullptr && dependencyCount != 0)
	{
		create_inf.dependencyCount = dependencyCount;
		create_inf.pDependencies = dependency;
	}

	if (handle != nullptr)
	{
		int a = 5;
		_isFailed = false;
	}
	else
	{
		if (VK_SUCCESS != vkCreateRenderPass(dev, &create_inf, nullptr, &handle))
		{
			_isFailed = true;
		}
		else
		{
			_isFailed = false;
		}
	}
	
	if (!_isFailed)
	{
		_info.resize(1);

		VkFramebufferCreateInfo fb_info = {};
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_info.pNext = nullptr;

		fb_info.renderPass = handle;
		fb_info.attachmentCount = 1;
		fb_info.width = width;
		fb_info.height = height;
		fb_info.layers = 1;
		fb_info.pAttachments = &imageView;
		VkFramebuffer frameBuffer;
		if (vkCreateFramebuffer(dev, &fb_info, nullptr, &frameBuffer) != VK_SUCCESS)
		{
			_isFailed = true;
		}
		else
		{
			_isFailed = false;
			_info[0].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			_info[0].renderPass = handle;
			_info[0].framebuffer = frameBuffer;
			_info[0].renderArea.extent.width = width;
			_info[0].renderArea.extent.height = height;
			_info[0].clearValueCount = (uint32_t)_clearValues.size();
			_info[0].pClearValues = _clearValues.data();
		}
	}
}

void GVulkanRenderpass::destroy(VkDevice dev,bool forResize)
{	
	if (_info[0].renderPass != nullptr && !forResize)
	{
		vkDestroyRenderPass(dev, _info[0].renderPass, nullptr);
		_info[0].renderPass = nullptr;
	}

	for (int i = 0; i < _info.size(); i++)
	{
		if (_info[i].framebuffer != nullptr)
		{
			vkDestroyFramebuffer(dev, _info[i].framebuffer, nullptr);
			_info[i].framebuffer = nullptr;
		}
	}
	_isFailed = true;

}

VkRenderPass_T* GVulkanRenderpass::get_vk_renderpass()
{
	return get_handle();
}
