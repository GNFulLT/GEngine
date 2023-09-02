
#include "internal/engine/rendering/vulkan/gvulkan_vertex_state.h"

GVulkanVertexState::GVulkanVertexState()
{
	m_createInfo = {};
	m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_createInfo.pNext = nullptr;
	m_createInfo.flags = 0;
	m_createInfo.vertexBindingDescriptionCount = 0;
	m_createInfo.pVertexBindingDescriptions = 0;
	m_createInfo.vertexAttributeDescriptionCount = 0;
	m_createInfo.pVertexAttributeDescriptions = 0;
}
GVulkanVertexState::~GVulkanVertexState()
{
	int a = 5;
}

GVulkanVertexState::GVulkanVertexState(const std::vector< VkVertexInputBindingDescription>& vertexBindingDescription, const std::vector< VkVertexInputAttributeDescription>& attributeDescription)
{
	m_vertexBindingDescription = vertexBindingDescription;
	m_vertexAttributeDescription = attributeDescription;

	m_createInfo = {};
	m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_createInfo.pNext = nullptr;
	m_createInfo.flags = 0;
	m_createInfo.vertexBindingDescriptionCount = m_vertexBindingDescription.size();
	m_createInfo.pVertexBindingDescriptions = m_vertexBindingDescription.data();
	m_createInfo.vertexAttributeDescriptionCount = m_vertexAttributeDescription.size();
	m_createInfo.pVertexAttributeDescriptions = m_vertexAttributeDescription.data();
}

GRAPHIC_PIPELINE_STATE GVulkanVertexState::get_pipeline_state()
{
	return GRAPHIC_PIPELINE_STATE_VERTEX_INPUT;
}

void GVulkanVertexState::fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo)
{
	createInfo->pVertexInputState = &m_createInfo;
}
