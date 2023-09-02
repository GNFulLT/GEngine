#include "internal/engine/rendering/vulkan/gvulkan_viewport_state.h"

GVulkanViewportState::GVulkanViewportState(uint32_t width,uint32_t height)
{
	m_viewport.width = width;
	m_viewport.height = height;
	m_viewport.x = 0;
	m_viewport.y = 0;
	m_viewport.minDepth = 0;
	m_viewport.maxDepth = 1;

	m_scissor.offset = { 0, 0 };
	m_scissor.extent.width = width;
	m_scissor.extent.height = height;

	m_createInfo = {};
	m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	m_createInfo.pNext = nullptr;
	m_createInfo.viewportCount = 1;
	m_createInfo.pViewports = &m_viewport;
	m_createInfo.scissorCount = 1;
	m_createInfo.pScissors = &m_scissor;
}

GRAPHIC_PIPELINE_STATE GVulkanViewportState::get_pipeline_state()
{
	return GRAPHIC_PIPELINE_STATE_VIEWPORT;
}

void GVulkanViewportState::fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo)
{
	createInfo->pViewportState = &m_createInfo;
}
