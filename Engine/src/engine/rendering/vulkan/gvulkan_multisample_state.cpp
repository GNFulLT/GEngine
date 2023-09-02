#include "internal/engine/rendering/vulkan/gvulkan_multisample_state.h"

GVulkanMultiSampleState::GVulkanMultiSampleState()
{
	m_createInfo = {};
	m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	m_createInfo.pNext = nullptr;

	m_createInfo.sampleShadingEnable = VK_FALSE;
	//multisampling defaulted to no multisampling (1 sample per pixel)
	m_createInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	m_createInfo.minSampleShading = 1.0f;
	m_createInfo.pSampleMask = nullptr;
	m_createInfo.alphaToCoverageEnable = VK_FALSE;
	m_createInfo.alphaToOneEnable = VK_FALSE;
}

GRAPHIC_PIPELINE_STATE GVulkanMultiSampleState::get_pipeline_state()
{
	return GRAPHIC_PIPELINE_STATE_MULTISAMPLE;
}

void GVulkanMultiSampleState::fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo)
{
	createInfo->pMultisampleState = &m_createInfo;
}
