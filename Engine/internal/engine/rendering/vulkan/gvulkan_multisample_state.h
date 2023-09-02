#ifndef GVULKAN_MULTISAMPLE_STATE_H
#define GVULKAN_MULTISAMPLE_STATE_H


#include "volk.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"

class GVulkanMultiSampleState : public IGVulkanGraphicPipelineState
{
public:
	GVulkanMultiSampleState();
	// Inherited via IGVulkanGraphicPipelineState
	virtual GRAPHIC_PIPELINE_STATE get_pipeline_state() override;
	virtual void fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo) override;
private:
	VkPipelineMultisampleStateCreateInfo m_createInfo;
};


#endif // GVULKAN_MULTISAMPLE_STATE_H