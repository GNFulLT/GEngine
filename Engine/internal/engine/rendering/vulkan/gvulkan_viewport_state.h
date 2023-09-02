#ifndef GVULKAN_VIEWPORT_STATE_H
#define GVULKAN_VIEWPORT_STATE_H



#include "volk.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"

class GVulkanViewportState : public IGVulkanGraphicPipelineState
{
public:
	GVulkanViewportState(uint32_t width, uint32_t height);

	// Inherited via IGVulkanGraphicPipelineState
	virtual GRAPHIC_PIPELINE_STATE get_pipeline_state() override;
	virtual void fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo) override;
private:
	VkPipelineViewportStateCreateInfo m_createInfo;
	VkViewport m_viewport;
	VkRect2D m_scissor;
	
};
#endif //GVULKAN_VIEWPORT_STATE_H