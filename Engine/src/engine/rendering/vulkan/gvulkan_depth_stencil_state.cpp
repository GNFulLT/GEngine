#include "internal/engine/rendering/vulkan/gvulkan_depth_stencil_state.h"

GVulkanDepthStencilState::GVulkanDepthStencilState()
{
	info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.depthTestEnable = VK_TRUE;
	info.depthWriteEnable = VK_TRUE;
	info.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	info.stencilTestEnable = false;
	info.minDepthBounds = 0;
	info.maxDepthBounds = 1;
}

GRAPHIC_PIPELINE_STATE GVulkanDepthStencilState::get_pipeline_state()
{
	return GRAPHIC_PIPELINE_STATE_DEPTH_STENCIL;
}

void GVulkanDepthStencilState::fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo)
{
	createInfo->pDepthStencilState = &info;
}
