#include "internal/engine/rendering/vulkan/gvulkan_rasterization_state.h"

GVulkanRasterizationState::GVulkanRasterizationState()
{
	m_createInfo = {};
	m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	m_createInfo.depthClampEnable = VK_FALSE;
	//discards all primitives before the rasterization stage if enabled which we don't want
	m_createInfo.rasterizerDiscardEnable = VK_FALSE;

	m_createInfo.polygonMode = VK_POLYGON_MODE_FILL;
	m_createInfo.lineWidth = 1.0f;
	//no backface cull
	m_createInfo.cullMode = VK_CULL_MODE_NONE;
	m_createInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	//no depth bias
	m_createInfo.depthBiasEnable = VK_FALSE;
	m_createInfo.depthBiasConstantFactor = 0.0f;
	m_createInfo.depthBiasClamp = 0.0f;
	m_createInfo.depthBiasSlopeFactor = 0.0f;

}

GVulkanRasterizationState::GVulkanRasterizationState(const VkPipelineRasterizationStateCreateInfo* inf)
{
	m_createInfo = *inf;
}

GRAPHIC_PIPELINE_STATE GVulkanRasterizationState::get_pipeline_state()
{
	return GRAPHIC_PIPELINE_STATE_RASTERIZATION;
}

void GVulkanRasterizationState::fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo)
{
	createInfo->pRasterizationState = &m_createInfo;
}
