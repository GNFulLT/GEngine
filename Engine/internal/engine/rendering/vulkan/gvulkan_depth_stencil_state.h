#ifndef GVULKAN_DEPTH_STENCIL_STATE_H
#define GVULKAN_DEPTH_STENCIL_STATE_H


#include "volk.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"

class GVulkanDepthStencilState : public IGVulkanGraphicPipelineState
{
public:
	GVulkanDepthStencilState();

	// Inherited via IGVulkanGraphicPipelineState
	virtual GRAPHIC_PIPELINE_STATE get_pipeline_state() override;
	virtual void fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo) override;

private:
	VkPipelineDepthStencilStateCreateInfo info;

};

#endif // GVULKAN_DEPTH_STENCIL_STATE_H