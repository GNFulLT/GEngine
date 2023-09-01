#ifndef GVULKAN_RENDERPASS_H
#define GVULKAN_RENDERPASS_H

#include "public/plugin/c_vec2.h"

#include <volk.h>
#include <vector>

#include "engine/rendering/vulkan/ivulkan_renderpass.h"

class GVulkanRenderpass : public IGVulkanRenderPass
{
public:
	GVulkanRenderpass();

	void create(VkDevice dev, const std::vector<VkImageView>& views, std::vector<C_GVec2>& sizes, const std::vector<VkClearValue>& clearValues,
		VkFormat format, VkImageLayout finalLayout, VkImageLayout attachmentReferenceLayout, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE);
	void create(VkDevice dev, VkImageView imageView, uint32_t width, uint32_t height, const std::vector<VkClearValue>& clearValues,
		VkFormat format, VkImageLayout finalLayout, VkImageLayout attachmentReferenceLayout, VkSubpassContents subpassContents = VK_SUBPASS_CONTENTS_INLINE, VkSubpassDependency* dependencies = 0,int dependencyCount = 0);

	void destroy(VkDevice dev);

	inline void begin(VkCommandBuffer cmd, int index = 0)
	{
		vkCmdBeginRenderPass(cmd, &_info[index], _subpassContents);
	}

	inline void end(VkCommandBuffer cmd)
	{
		vkCmdEndRenderPass(cmd);
	}

	inline void set_size(const C_GVec2* size, int index = 0)
	{
		_info[index].renderArea.extent.width = (uint32_t)size->x;
		_info[index].renderArea.extent.height = (uint32_t)size->y;
	}

	inline bool is_failed() const noexcept
	{
		return _isFailed;
	}

	inline VkRenderPass get_handle() const
	{
		return _info[0].renderPass;
	}

	inline VkRenderPass get_handle(uint32_t index) const
	{
		return _info[index].renderPass;
	}

	// Inherited via IGVulkanRenderPass
	virtual VkRenderPass_T* get_vk_renderpass() override;
private:
	std::vector<VkRenderPassBeginInfo> _info;
	VkSubpassContents _subpassContents;
	std::vector<VkClearValue> _clearValues;
	std::vector<VkPipelineStageFlags> _stageFlags;
	bool _isFailed = false;



};

#endif // VULKAN_RENDERPASS_H