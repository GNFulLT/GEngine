#include "internal/engine/rendering/vulkan/gvulkan_graphic_pipeline_custom_layout.h"

#include "internal/engine/rendering/vulkan/gvulkan_graphic_pipeline.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"
#include "engine/rendering/vulkan/ivulkan_renderpass.h"
#include "engine/rendering/vulkan/ivulkan_pipeline_layout.h"
#include "engine/resource/igshader_resource.h"
#include <unordered_map>
#include <cassert>
#include "engine/rendering/vulkan/ivulkan_descriptor_pool.h"
#include "engine/rendering/vulkan/ivulkan_swapchain.h"
#include "engine/gengine.h"
#include "engine/rendering/vulkan/ivulkan_shader_info.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"

GVulkanGraphicPipelineCustomLayout::GVulkanGraphicPipelineCustomLayout(IGVulkanLogicalDevice* dev, IGVulkanRenderPass* boundedRenderpass, const std::vector<IVulkanShaderStage*>& shaderStages, const std::vector<IGVulkanGraphicPipelineState*>& states, IGVulkanGraphicPipelineLayoutCreator* creator, int flag)
{
	m_boundedDevice = dev;
	m_boundedRenderpass = boundedRenderpass;
	m_shaderStages = shaderStages;
	m_pipelineStates = states;
	m_layoutCreator = creator;
	m_flag = flag;
}

bool GVulkanGraphicPipelineCustomLayout::init()
{
	auto res2 = m_layoutCreator->create_descriptor_pool_and_sets(this,&m_descriptorSets);
	if (!res2.has_value())
	{
		return false;
	}

	m_graphicPool = res2.value();

	auto res = m_layoutCreator->create_layout_for(this);
	if (!res.has_value())
	{
		return false;
	}

	m_pipelineLayout = res.value();
	
	VkGraphicsPipelineCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	info.flags = m_flag;

	//X TODO STATIC : 
	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT ,VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;
	dynamicState.flags = 0;

	info.pDynamicState = &dynamicState;

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
	info.layout = m_pipelineLayout->get_vk_pipeline_layout();
	info.basePipelineHandle = VK_NULL_HANDLE;
	info.basePipelineIndex = -1;

	m_layoutCreator->inject_create_info(&info);

	auto resVk = vkCreateGraphicsPipelines(m_boundedDevice->get_vk_device(), VK_NULL_HANDLE, 1, &info, nullptr, &m_vkPipeline);

	return resVk == VK_SUCCESS;


}

IGVulkanPipelineLayout* GVulkanGraphicPipelineCustomLayout::get_pipeline_layout()
{
	return m_pipelineLayout;
}

const std::vector<IVulkanShaderStage*>* GVulkanGraphicPipelineCustomLayout::get_shader_stages()
{
	return &m_shaderStages;
}

VkPipeline_T* GVulkanGraphicPipelineCustomLayout::get_pipeline()
{
	return m_vkPipeline;
}

void GVulkanGraphicPipelineCustomLayout::destroy()
{
	if (m_layoutCreator != nullptr)
	{
		m_layoutCreator->destroy();
		delete m_layoutCreator;
		m_layoutCreator = nullptr;
	}

	if (m_pipelineLayout != nullptr)
	{
		m_pipelineLayout->destroy();
		m_pipelineLayout = nullptr;
	}
	if (m_vkPipeline != nullptr)
	{
		vkDestroyPipeline(m_boundedDevice->get_vk_device(), m_vkPipeline, nullptr);
		m_vkPipeline = nullptr;
	}
	if (m_graphicPool != nullptr)
	{
		m_graphicPool->destroy();
		m_graphicPool = nullptr;

	}
}

void GVulkanGraphicPipelineCustomLayout::bind_sets(GVulkanCommandBuffer* cmd, uint32_t frameIndex)
{
	vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout->get_vk_pipeline_layout(), 0, 1, &m_descriptorSets[frameIndex], 0, nullptr);
}


