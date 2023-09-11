#ifndef GVULKAN_COLOR_BLEND_STATE_H
#define GVULKAN_COLOR_BLEND_STATE_H



#include "volk.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"

class GVulkanColorBlendState : public IGVulkanGraphicPipelineState
{
public:
	GVulkanColorBlendState();
	GVulkanColorBlendState(const VkPipelineColorBlendAttachmentState* attachment,const VkPipelineColorBlendStateCreateInfo* inf);
	virtual GRAPHIC_PIPELINE_STATE get_pipeline_state() override;
	virtual void fill_pipeline_create_info(VkGraphicsPipelineCreateInfo* createInfo) override;
private:
	VkPipelineColorBlendStateCreateInfo m_createInfo;
	VkPipelineColorBlendAttachmentState m_attachmentState;
};
#endif // GVULKAN_COLOR_BLEND_STATE_H