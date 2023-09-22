#include "volk.h"
#include "internal/engine/rendering/renderer/gscene_wireframe_renderer_layout.h"
#include <unordered_map>
#include <array>
#include <cassert>
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"
#include "engine/rendering/wireframe_spec.h"
#include "internal/engine/rendering/mesh/gmesh_renderable.h"

GSceneWireframeRendererLayout::GSceneWireframeRendererLayout(IGVulkanLogicalDevice* dev, uint32_t framesInFlight, IGSceneManager* sceneManager, IVulkanBuffer* meshBuffer, IVulkanBuffer* transformBuffer, IVulkanBuffer* drawDataBuffer, IVulkanBuffer* drawDataIDBuffer,
	IVulkanBuffer* materialBuffer)
{
	p_transformBuffer = transformBuffer;
	p_drawDataIDuffer = drawDataIDBuffer;
	p_boundedDevice = dev;
	m_framesInFlight = framesInFlight;
	p_sceneManager = sceneManager;
	p_meshBuffer = meshBuffer;
	p_drawDataBuffer = drawDataBuffer;
	p_materialBuffer = materialBuffer;
}

void GSceneWireframeRendererLayout::inject_create_info(VkGraphicsPipelineCreateInfo* info)
{
}

std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> GSceneWireframeRendererLayout::create_layout_for(IGVulkanGraphicPipeline* pipeline)
{
	VkPushConstantRange range = {};
	range.offset = 0;
	range.size = sizeof(WireFrameSpec);
	range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineLayoutCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	inf.flags = 0;
	inf.setLayoutCount = 1;
	inf.pSetLayouts = &m_descriptorSetLayout;
	inf.pushConstantRangeCount = 1;
	inf.pPushConstantRanges = &range;

	VkPipelineLayout layout;
	auto res = vkCreatePipelineLayout(p_boundedDevice->get_vk_device(), &inf, nullptr, &layout);
	assert(res == VK_SUCCESS);

	m_pipelineLayout = new GVulkanBasicPipelineLayout(p_boundedDevice, layout);
	return m_pipelineLayout;
}

std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> GSceneWireframeRendererLayout::create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets)
{
	p_descriptorSets = descriptorSets;

	//X First create necessary pool
	std::unordered_map<VkDescriptorType, int> map;
	map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
	map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 5);

	auto pool = p_boundedDevice->create_and_init_vector_pool(map, m_framesInFlight);

	//X Now create descriptor set layout

	std::array<VkDescriptorSetLayoutBinding, 6> bindings;
	//X Cam Buff
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	//X MESH DATA  BUFF
	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[1].pImmutableSamplers = nullptr;

	//X DRAW DATA BUFF
	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[2].pImmutableSamplers = nullptr;

	//X Draw Data ID Buffer
	bindings[3].binding = 3;
	bindings[3].descriptorCount = 1;
	bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[3].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[3].pImmutableSamplers = nullptr;


	//X Material Buff
	bindings[4].binding = 4;
	bindings[4].descriptorCount = 1;
	bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[4].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[4].pImmutableSamplers = nullptr;

	//X Transform Buff
	bindings[5].binding = 5;
	bindings[5].descriptorCount = 1;
	bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[5].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[5].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pNext = nullptr;
	//we are going to have 1 binding
	setinfo.bindingCount = bindings.size();
	//no flags
	setinfo.flags = 0;
	//point to the camera buffer binding
	setinfo.pBindings = bindings.data();

	auto res = vkCreateDescriptorSetLayout(p_boundedDevice->get_vk_device(), &setinfo, nullptr, &m_descriptorSetLayout);
	assert(res == VK_SUCCESS);

	std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight, m_descriptorSetLayout);

	//allocate one descriptor set for each frame
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool->get_vk_descriptor_pool();
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	std::vector<VkDescriptorSet> vkdescriptorSets;
	vkdescriptorSets.resize(m_framesInFlight);

	assert(VK_SUCCESS == vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &allocInfo, vkdescriptorSets.data()));

	for (int i = 0; i < vkdescriptorSets.size(); i++)
	{
		auto gbuff = p_sceneManager->get_global_buffer_for_frame(i);
		std::array<VkDescriptorBufferInfo, 6> bufferInfos;
		//it will be the camera buffer
		bufferInfos[0].buffer = gbuff->get_vk_buffer();
		bufferInfos[0].offset = 0;
		bufferInfos[0].range = gbuff->get_size();
		
		//X Mesh Data
		bufferInfos[1].buffer = p_meshBuffer->get_vk_buffer();
		bufferInfos[1].offset = 0;
		bufferInfos[1].range = p_meshBuffer->get_size();

		//X Draw Data
		bufferInfos[2].buffer = p_drawDataBuffer->get_vk_buffer();
		bufferInfos[2].offset = 0;
		bufferInfos[2].range = p_drawDataBuffer->get_size();

		//X DrawID Data
		bufferInfos[3].buffer = p_drawDataIDuffer->get_vk_buffer();
		bufferInfos[3].offset = 0;
		bufferInfos[3].range = p_drawDataIDuffer->get_size();

		//X Material Data
		bufferInfos[4].buffer = p_materialBuffer->get_vk_buffer();
		bufferInfos[4].offset = 0;
		bufferInfos[4].range = p_materialBuffer->get_size();

		//X Transform Data
		bufferInfos[5].buffer = p_transformBuffer->get_vk_buffer();
		bufferInfos[5].offset = 0;
		bufferInfos[5].range = p_transformBuffer->get_size();

		std::array< VkWriteDescriptorSet, 6> setWrites;
		setWrites[0] = {};
		setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[0].pNext = nullptr;
		setWrites[0].dstBinding = 0;
		setWrites[0].dstSet = vkdescriptorSets[i];
		setWrites[0].descriptorCount = 1;
		setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setWrites[0].pBufferInfo = &(bufferInfos[0]);
		//-------------------

		//X Mesh Data
		setWrites[1] = {};
		setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[1].pNext = nullptr;
		setWrites[1].dstBinding = 1;
		setWrites[1].dstSet = vkdescriptorSets[i];
		setWrites[1].descriptorCount = 1;
		setWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[1].pBufferInfo = &(bufferInfos[1]);

		//X Draw Data
		setWrites[2] = {};
		setWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[2].pNext = nullptr;
		setWrites[2].dstBinding = 2;
		setWrites[2].dstSet = vkdescriptorSets[i];
		setWrites[2].descriptorCount = 1;
		setWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[2].pBufferInfo = bufferInfos.data() + 2;

		//X Draw ID Data
		setWrites[3] = {};
		setWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[3].pNext = nullptr;
		setWrites[3].dstBinding = 3;
		setWrites[3].dstSet = vkdescriptorSets[i];
		setWrites[3].descriptorCount = 1;
		setWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[3].pBufferInfo = bufferInfos.data() + 3;

		//X Material Data
		setWrites[4] = {};
		setWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[4].pNext = nullptr;
		setWrites[4].dstBinding = 4;
		setWrites[4].dstSet = vkdescriptorSets[i];
		setWrites[4].descriptorCount = 1;
		setWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[4].pBufferInfo = bufferInfos.data() + 4;

		//X Transform Data
		setWrites[5] = {};
		setWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[5].pNext = nullptr;
		setWrites[5].dstBinding = 5;
		setWrites[5].dstSet = vkdescriptorSets[i];
		setWrites[5].descriptorCount = 1;
		setWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[5].pBufferInfo = bufferInfos.data() + 5;

		vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), setWrites.size(), setWrites.data(), 0, nullptr);

	}

	(*descriptorSets) = vkdescriptorSets;
	return pool;
}

VkDescriptorSetLayout_T* GSceneWireframeRendererLayout::get_set_layout()
{
	return m_descriptorSetLayout;
}

std::vector<VkDescriptorSet_T*>* GSceneWireframeRendererLayout::get_sets()
{
	return p_descriptorSets;
}

void GSceneWireframeRendererLayout::write_set_layout(uint32_t binding, uint32_t bufferType, VkDescriptorBufferInfo* info)
{
	for (int i = 0; i < m_framesInFlight; i++)
	{
		VkWriteDescriptorSet setWrites;
		setWrites = {};
		setWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites.pNext = nullptr;
		setWrites.dstBinding = binding;
		setWrites.dstSet = (*p_descriptorSets)[i];
		setWrites.descriptorCount = 1;
		setWrites.descriptorType = (VkDescriptorType)bufferType;
		setWrites.pBufferInfo = info;
		
		vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), 1, &setWrites, 0, nullptr);

	}
}
