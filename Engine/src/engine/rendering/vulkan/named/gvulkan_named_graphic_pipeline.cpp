#include "internal/engine/rendering/vulkan/named/gvulkan_named_graphic_pipeline.h"
#include "engine/rendering/vulkan/ivulkan_shader_stage.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"

GVulkanNamedGraphicPipeline::GVulkanNamedGraphicPipeline(IGVulkanLogicalDevice* dev, IGVulkanNamedRenderPass* boundedRenderpass,const char* name)
{
	p_boundedDevice = dev;
	m_name = name;
	p_boundedRenderpass = boundedRenderpass;
	m_pipeline = nullptr;
}

bool GVulkanNamedGraphicPipeline::init(IGVulkanNamedPipelineLayout* layout, const std::vector<IVulkanShaderStage*>& shaderStages, const std::vector<IGVulkanGraphicPipelineState*>& states)
{
	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	info.flags = 0;

	//X TODO STATIC : 
	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT ,VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;
	dynamicState.flags = 0;

	info.pDynamicState = &dynamicState;

	std::vector<VkPipelineShaderStageCreateInfo> stages(shaderStages.size());
	for (int i = 0; i < shaderStages.size(); i++)
	{
		stages[i] = (*shaderStages[i]->get_creation_info());
	}

	info.stageCount = stages.size();
	info.pStages = stages.data();

	for (int i = 0; i < states.size(); i++)
	{
		states[i]->fill_pipeline_create_info(&info);
	}

	info.renderPass = p_boundedRenderpass->get_vk_renderpass();
	info.layout = layout->get_vk_pipeline_layout();
	info.basePipelineHandle = VK_NULL_HANDLE;
	info.basePipelineIndex = -1;

	auto resVk = vkCreateGraphicsPipelines(p_boundedDevice->get_vk_device(), VK_NULL_HANDLE, 1, &info, nullptr, &m_pipeline);

	return resVk == VK_SUCCESS;

}

void GVulkanNamedGraphicPipeline::destroy()
{
	vkDestroyPipeline(p_boundedDevice->get_vk_device(), m_pipeline, nullptr);
}

VkPipeline_T* GVulkanNamedGraphicPipeline::get_vk_pipeline() const noexcept
{
	return m_pipeline;
}
