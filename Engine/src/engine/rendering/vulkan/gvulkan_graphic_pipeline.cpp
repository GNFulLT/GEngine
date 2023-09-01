#include "internal/engine/rendering/vulkan/gvulkan_graphic_pipeline.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"
#include "engine/rendering/vulkan/ivulkan_renderpass.h"
#include "engine/rendering/vulkan/ivulkan_pipeline_layout.h"

GVulkanGraphicPipeline::GVulkanGraphicPipeline(IGVulkanLogicalDevice* dev, IGVulkanRenderPass* boundedRenderpass, IGVulkanPipelineLayout* boundedLayout,const std::vector<IVulkanShaderStage*>& shaderStages, const std::vector<IGVulkanGraphicPipelineState*>& states, int flag)
{
	m_shaderStages = shaderStages;
	m_pipelineStates = states;
	m_flag = flag;
	m_vkPipeline = nullptr;
	m_boundedDevice = dev;
	m_boundedLayout = boundedLayout;
	m_boundedRenderpass = boundedRenderpass;
}

bool GVulkanGraphicPipeline::init()
{
	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	info.flags = m_flag;

	std::vector<VkPipelineShaderStageCreateInfo> stages(m_shaderStages.size());
	for (int i = 0; i < m_shaderStages.size(); i++)
	{
		stages[i] = (*m_shaderStages[i]->get_creation_info());
	}

	info.stageCount = stages.size();
	info.pStages = stages.data();
	
	for (int i = 0; i < m_pipelineStates.size(); i++)
	{
		m_pipelineStates[i]->fill_pipeline_create_info(&info);
	}

	info.renderPass = m_boundedRenderpass->get_vk_renderpass();
	info.layout = m_boundedLayout->get_vk_pipeline_layout();
	info.basePipelineHandle = VK_NULL_HANDLE;
	info.basePipelineIndex = -1;

	auto res = vkCreateGraphicsPipelines(m_boundedDevice->get_vk_device(), VK_NULL_HANDLE,1,&info,nullptr,&m_vkPipeline);
	
	return res == VK_SUCCESS;
}

void GVulkanGraphicPipeline::destroy()
{
	if (m_vkPipeline != nullptr)
	{
		vkDestroyPipeline(m_boundedDevice->get_vk_device(), m_vkPipeline, nullptr);
	}
}

const std::vector<IVulkanShaderStage*>* GVulkanGraphicPipeline::get_shader_stages()
{
	return &m_shaderStages;
}
