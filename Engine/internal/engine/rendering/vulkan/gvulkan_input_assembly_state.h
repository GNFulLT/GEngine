#ifndef GVULKAN_INPUT_ASSEMBLY_STATE_H
#define GVULKAN_INPUT_ASSEMBLY_STATE_H

#include "volk.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"

class GVulkanInputAssemblyState : public IGVulkanGraphicPipelineState
{
public:
	GVulkanInputAssemblyState();

	GVulkanInputAssemblyState(VkPrimitiveTopology topology,bool resetAfterIndexed);

	// Inherited via IGVulkanGraphicPipelineState
	virtual GRAPHIC_PIPELINE_STATE get_pipeline_state() override;
	virtual void fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo) override;
private:
	VkPipelineInputAssemblyStateCreateInfo m_createInfo;
};

#endif // GVULKAN_INPUT_ASSEMBLY_STATE_H