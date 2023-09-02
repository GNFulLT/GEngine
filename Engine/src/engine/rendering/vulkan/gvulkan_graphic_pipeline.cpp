#include "volk.h"

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
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"

GVulkanGraphicPipeline::GVulkanGraphicPipeline(IGVulkanLogicalDevice* dev, IGVulkanRenderPass* boundedRenderpass,const std::vector<IVulkanShaderStage*>& shaderStages, const std::vector<IGVulkanGraphicPipelineState*>& states, int flag)
{
	m_graphicPool = nullptr;
	m_shaderStages = shaderStages;
	m_pipelineStates = states;
	m_flag = flag;
	m_vkPipeline = nullptr;
	m_boundedDevice = dev;
	m_boundedRenderpass = boundedRenderpass;
}

bool GVulkanGraphicPipeline::init()
{
	std::unordered_map<VkDescriptorType, int> types;
	std::vector<VkDescriptorSetLayout_T*> layouts;
	for (int i = 0; i < GEngine::get_instance()->get_swapchain()->get_total_image(); i++)
	{
		m_writeSets.emplace(i, std::vector<VkWriteDescriptorSet>());
	}

	for (int i = 0; i < m_shaderStages.size(); i++)
	{
		auto info  = m_shaderStages[i]->get_shader_resource()->get_shader_info();
		if (info == nullptr)
			continue;
		for (int j = 0; j < info->get_binding_count(); j++)
		{
			auto binding = info->get_binding_by_index(i);
			if (binding != nullptr)
			{
				if (auto type = types.find(binding->descriptorType); type != types.end())
				{
					types[binding->descriptorType]++;
				}
				else
				{
					types.emplace(binding->descriptorType, 1);
				}


				//X Create the write sets
				for (int t = 0; t < GEngine::get_instance()->get_swapchain()->get_total_image(); t++)
				{
					auto writeSet = *info->get_write_set_by_index(j);
					writeSet.dstSet = m_descriptorSets[i];
					m_writeSets[i].push_back(writeSet);
				}
			}
		}


		auto set = m_shaderStages[i]->get_shader_resource()->get_layout_set();
		if (set != nullptr)
		{
			layouts.push_back(set);
		}
	}

	//X Shaders have some bindings. Create layouts and descriptors for them

	if (layouts.size() > 0)
	{
		m_graphicPool = m_boundedDevice->create_and_init_vector_pool(types);

		if (m_graphicPool == nullptr)
		{
			return false;
		}

		//X CREATED THE POOL NOW ALLOCATE DESCRIPTORS

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_graphicPool->get_vk_descriptor_pool();
		allocInfo.descriptorSetCount = GEngine::get_instance()->get_swapchain()->get_total_image();
		allocInfo.pSetLayouts = layouts.data();

		m_descriptorSets.resize(GEngine::get_instance()->get_swapchain()->get_total_image());

		auto descriptorSetRes = vkAllocateDescriptorSets(m_boundedDevice->get_vk_device(), &allocInfo, m_descriptorSets.data());

		if (descriptorSetRes != VK_SUCCESS)
		{
			return false;
		}

		//X Now create the write sets for decsriptor sets
		for (int i = 0; i < m_shaderStages.size(); i++)
		{
			auto info = m_shaderStages[i]->get_shader_resource()->get_shader_info();
			for (int j = 0; j < info->get_binding_count(); j++)
			{
				for (int t = 0; t < GEngine::get_instance()->get_swapchain()->get_total_image(); t++)
				{
					auto writeSet = *info->get_write_set_by_index(j);
					writeSet.dstSet = m_descriptorSets[i];
					m_writeSets[i].push_back(writeSet);
				}
			}
		}


	}
	

	//X TODO : PARTIAL UPDATE
	//X Create the buffers storage uniform and sampler

	//for (int i = 0; i < m_descriptorSets.size(); i++)
	//{
	//	auto set = m_descriptorSets[i];
	//	// First create for uniform buffers
	//	std::vector<VkDescriptorBufferInfo> bufferInfos;
	//	std::vector<VkWriteDescriptorSet> writeSets;
	//	std::vector<uint32_t> bufferWriteIndex;
	//	for (int j = 0; m_uniformBuffers.size(); j++)
	//	{
	//		VkDescriptorBufferInfo info = {};
	//		info.buffer = m_uniformBuffers[i]->get_vk_buffer();
	//		info.offset = 0;
	//		info.range = m_uniformBuffers[i]->get_size();

	//		bufferInfos.push_back(info);
	//		int bufferIndex = -1;
	//		for (int t = 0; t < uniformSets.size(); t++)
	//		{
	//			if (uniformSets[t].first == m_uniformBuffers[j])
	//			{
	//				bufferIndex = t;
	//				break;
	//			}
	//		}
	//		assert(bufferIndex != -1);
	//		bufferWriteIndex.push_back(bufferIndex);
	//		writeSets.push_back(uniformSets[bufferIndex].second);
	//	}

	//	// We took the all uniform buffers
	//	// Now create the sets
	//	for (int j = 0; j < writeSets.size(); j++)
	//	{
	//		auto write = &writeSets[bufferWriteIndex[j]];
	//		write->dstSet = m_descriptorSets[i];
	//		write->pBufferInfo = &bufferInfos[j];
	//	}

	//	vkUpdateDescriptorSets(m_boundedDevice->get_vk_device(), static_cast<uint32_t>(uniformSets.size()), writeSets.data(), 0, nullptr);
	//}
	// VKWRITE

	
	m_pipelineLayout = m_boundedDevice->create_and_init_vector_pipeline_layout(layouts);

	if (m_pipelineLayout == nullptr)
	{
		return false;
	}

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

	auto res = vkCreateGraphicsPipelines(m_boundedDevice->get_vk_device(), VK_NULL_HANDLE,1,&info,nullptr,&m_vkPipeline);
	
	return res == VK_SUCCESS;
}

void GVulkanGraphicPipeline::destroy()
{
	
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

const std::vector<IVulkanShaderStage*>* GVulkanGraphicPipeline::get_shader_stages()
{
	return &m_shaderStages;
}

VkPipeline_T* GVulkanGraphicPipeline::get_pipeline()
{
	return m_vkPipeline;
}
