#include "internal/engine/rendering/vulkan/gvulkan_render_target.h"

bool GVulkanRenderTarget::init(VkDevice dev, const std::vector<VkImageView>& renderViews, const std::vector<VkImageView>& depthViews, 
	const std::vector<VkClearValue>& clearValues, uint32_t width, uint32_t height, VkFormat format, VkImageLayout finalLayout, VkImageLayout attachmentReferenceLayout,
	IGVulkanRenderPass* pass, VkSubpassContents subpassContents)
{
	m_renderpass = pass;
	_subpassContents = subpassContents;
	_clearValues = clearValues;


	_info.resize(renderViews.size());
	for (int i = 0; i < renderViews.size(); i++)
	{
		VkFramebufferCreateInfo fb_info = {};
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_info.pNext = nullptr;

		VkImageView viewsWithDepth[] = { renderViews[i],depthViews[i] };

		fb_info.renderPass = m_renderpass->get_vk_renderpass();
		fb_info.attachmentCount = 2;
		fb_info.width = width;
		fb_info.height = height;
		fb_info.layers = 1;
		fb_info.pAttachments = viewsWithDepth;
		VkFramebuffer frameBuffer;
		if (vkCreateFramebuffer(dev, &fb_info, nullptr, &frameBuffer) != VK_SUCCESS)
		{
			return false;
		}
		else
		{
			_info[i].sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			_info[i].renderPass = m_renderpass->get_vk_renderpass();
			_info[i].framebuffer = frameBuffer;
			_info[i].renderArea.extent.width = width;
			_info[i].renderArea.extent.height = height;
			_info[i].clearValueCount = (uint32_t)_clearValues.size();
			_info[i].pClearValues = _clearValues.data();
		}
	}
}

void GVulkanRenderTarget::destroy(VkDevice dev)
{
	for (int i = 0; i < _info.size(); i++)
	{
		if (_info[i].framebuffer != nullptr)
		{
			vkDestroyFramebuffer(dev, _info[i].framebuffer, nullptr);
			_info[i].framebuffer = nullptr;
		}
	}
}

VkRenderPass_T* GVulkanRenderTarget::get_vk_renderpass()
{
	return m_renderpass->get_vk_renderpass();
}
