#include "internal/engine/rendering/vulkan/gvulkan_color_blend_state.h"
#include <cassert>
GVulkanColorBlendState::GVulkanColorBlendState(const VkPipelineColorBlendAttachmentState* attachment, const VkPipelineColorBlendStateCreateInfo* inf)
{
	assert(inf->attachmentCount == 1);

	m_attachmentState = *attachment;
	m_createInfo = *inf;
	m_createInfo.pAttachments = &m_attachmentState;
}
GVulkanColorBlendState::GVulkanColorBlendState()
{
	m_attachmentState = {};
	m_attachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	m_attachmentState.blendEnable = VK_FALSE;

	m_createInfo = {};
	m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_createInfo.logicOpEnable = VK_FALSE;
	m_createInfo.logicOp = VK_LOGIC_OP_COPY;
	m_createInfo.attachmentCount = 1;
	m_createInfo.pAttachments = &m_attachmentState;

}

GRAPHIC_PIPELINE_STATE GVulkanColorBlendState::get_pipeline_state()
{
	return GRAPHIC_PIPELINE_STATE_COLOR_BLEND;
}

void GVulkanColorBlendState::fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo)
{
	createInfo->pColorBlendState = &m_createInfo;
}
