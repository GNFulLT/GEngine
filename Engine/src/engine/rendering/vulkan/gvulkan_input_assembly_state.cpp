#include "internal/engine/rendering/vulkan/gvulkan_input_assembly_state.h"

GVulkanInputAssemblyState::GVulkanInputAssemblyState()
{
	m_createInfo = {};
	m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_createInfo.flags = 0;
	m_createInfo.primitiveRestartEnable = false;
	m_createInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}

GVulkanInputAssemblyState::GVulkanInputAssemblyState(VkPrimitiveTopology topology, bool resetAfterIndexed)
{
	m_createInfo = {};
	m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	m_createInfo.flags = 0;
	m_createInfo.primitiveRestartEnable = resetAfterIndexed;
	m_createInfo.topology = topology;
}

GRAPHIC_PIPELINE_STATE GVulkanInputAssemblyState::get_pipeline_state()
{
	return GRAPHIC_PIPELINE_STATE_INPUT_ASSEMBLY;
}

void GVulkanInputAssemblyState::fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo)
{
	createInfo->pInputAssemblyState = &m_createInfo;
}
