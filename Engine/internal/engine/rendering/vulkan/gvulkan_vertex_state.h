#ifndef GVULKAN_VERTEX_STATE_H
#define GVULKAN_VERTEX_STATE_H

#include "volk.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"
#include <vector>

class GVulkanVertexState : public IGVulkanGraphicPipelineState
{
public:
	GVulkanVertexState();
	~GVulkanVertexState();
	GVulkanVertexState(const std::vector< VkVertexInputBindingDescription>& vertexBindingDescription,const std::vector< VkVertexInputAttributeDescription>& attributeDescription);
	// Inherited via IGVulkanGraphicPipelineState
	virtual GRAPHIC_PIPELINE_STATE get_pipeline_state() override;
	virtual void fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo) override;
private:
	VkPipelineVertexInputStateCreateInfo m_createInfo;
	std::vector< VkVertexInputBindingDescription> m_vertexBindingDescription;
	std::vector< VkVertexInputAttributeDescription> m_vertexAttributeDescription;

};
#endif // GVULKAN_VERTEX_STATE_H