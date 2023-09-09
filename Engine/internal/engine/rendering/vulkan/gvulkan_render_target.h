#ifndef GVULKAN_RENDER_TARGET_H
#define GVULKAN_RENDER_TARGET_H

#include <volk.h>
#include <vector>
#include "engine/rendering/vulkan/ivulkan_renderpass.h"


//X Change it to IGVulkanRenderTarget
class GVulkanRenderTarget : public IGVulkanRenderPass
{
public:
	bool init(VkDevice dev, const std::vector<VkImageView>& renderViews, const std::vector<VkImageView>& depthViews, const std::vector<VkClearValue>& clearValues, uint32_t width, uint32_t height, VkFormat format,
		VkImageLayout finalLayout, VkImageLayout attachmentReferenceLayout,IGVulkanRenderPass* pass , VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE);

	void destroy(VkDevice dev);

	inline void begin(VkCommandBuffer cmd, int index = 0)
	{
		vkCmdBeginRenderPass(cmd, &_info[index], _subpassContents);
	}

	inline void end(VkCommandBuffer cmd)
	{
		vkCmdEndRenderPass(cmd);
	}

	virtual VkRenderPass_T* get_vk_renderpass() override;

private:
	std::vector<VkRenderPassBeginInfo> _info;
	VkSubpassContents _subpassContents;
	std::vector<VkClearValue> _clearValues;
	std::vector<VkPipelineStageFlags> _stageFlags;

	IGVulkanRenderPass* m_renderpass;
};

#endif // GVULKAN_RENDER_TARGET_H