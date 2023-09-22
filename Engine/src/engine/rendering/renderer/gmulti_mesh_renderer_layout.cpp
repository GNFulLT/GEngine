#include "volk.h"
#include <unordered_map>
#include <array>
#include "internal/engine/rendering/renderer/gmulti_mesh_renderer_layout.h"
#include <vector>
#include <cassert>
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"
#include "public/math/gmat4.h"
#include "engine/rendering/wireframe_spec.h"

GMultiMeshRendererLayout::GMultiMeshRendererLayout(IGVulkanLogicalDevice* dev,
	IGSceneManager* scene,IVulkanBuffer* storageBuff, IVulkanBuffer* meshData,IVulkanBuffer* drawDataBuff, IVulkanBuffer* transformData,IVulkanBuffer* materialData, uint32_t vertSize, uint32_t indexSize, uint32_t frameCount)
{
	m_meshBuffer = meshData;
	m_sceneManager = scene;
	m_storageBuffer = storageBuff;
	m_drawDataBuffer = drawDataBuff;
	vertexSize = vertSize;
	this->indexSize = indexSize;
	m_boundedDevice = dev;
	m_frameCount = frameCount;
	m_materialBuffer = materialData;
	m_transformBuffer = transformData;
}

void GMultiMeshRendererLayout::inject_create_info(VkGraphicsPipelineCreateInfo* info)
{
}

std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> GMultiMeshRendererLayout::create_layout_for(IGVulkanGraphicPipeline* pipeline)
{

	//X Pipeline Layout
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
	auto res = vkCreatePipelineLayout(m_boundedDevice->get_vk_device(), &inf, nullptr, &layout);
	assert(res == VK_SUCCESS);

	m_pipelineLayout = new GVulkanBasicPipelineLayout(m_boundedDevice, layout);
	return m_pipelineLayout;
}

std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> GMultiMeshRendererLayout::create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets)
{
	//X First create necessary pool
	std::unordered_map<VkDescriptorType, int> map;
	map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
	map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4);
	
	auto pool = m_boundedDevice->create_and_init_vector_pool(map, m_frameCount);

	//X Now create descriptor set layout

	std::array<VkDescriptorSetLayoutBinding, 7> bindings;
	//X Cam Buff
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	//X VERTEX BUFF
	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[1].pImmutableSamplers = nullptr;

	//X INDEX BUFF
	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[2].pImmutableSamplers = nullptr;

	//X Draw Data
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

	//X Transform Datas
	bindings[5].binding = 5;
	bindings[5].descriptorCount = 1;
	bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[5].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[5].pImmutableSamplers = nullptr;

	//X Mesh Datas
	bindings[6].binding = 6;
	bindings[6].descriptorCount = 1;
	bindings[6].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[6].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[6].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pNext = nullptr;
	//we are going to have 1 binding
	setinfo.bindingCount = bindings.size();
	//no flags
	setinfo.flags = 0;
	//point to the camera buffer binding
	setinfo.pBindings = bindings.data();

	auto res = vkCreateDescriptorSetLayout(m_boundedDevice->get_vk_device(), &setinfo, nullptr, &m_descriptorSetLayout);
	assert(res == VK_SUCCESS);


	//X Descriptor Set Allocation


	std::vector<VkDescriptorSetLayout> layouts(m_frameCount, m_descriptorSetLayout);

	//allocate one descriptor set for each frame
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool->get_vk_descriptor_pool();
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	std::vector<VkDescriptorSet> vkdescriptorSets;
	vkdescriptorSets.resize(m_frameCount);

	assert(VK_SUCCESS == vkAllocateDescriptorSets(m_boundedDevice->get_vk_device(), &allocInfo, vkdescriptorSets.data()));

	for (int i = 0; i < vkdescriptorSets.size(); i++)
	{
		auto gbuff = m_sceneManager->get_global_buffer_for_frame(i);
		std::array<VkDescriptorBufferInfo, 7> bufferInfos;
		//it will be the camera buffer
		bufferInfos[0].buffer = gbuff->get_vk_buffer();
		//at 0 offset
		bufferInfos[0].offset = 0;
		//of the size of a camera data struct
		bufferInfos[0].range = gbuff->get_size();
		//X Vertex Data
		bufferInfos[1].buffer = m_storageBuffer->get_vk_buffer();
		bufferInfos[1].offset = 0;
		bufferInfos[1].range = vertexSize;

		//X Index Data
		bufferInfos[2].buffer = m_storageBuffer->get_vk_buffer();
		bufferInfos[2].offset = vertexSize;
		bufferInfos[2].range = indexSize;

		//X Draw Data
		bufferInfos[3].buffer = m_drawDataBuffer->get_vk_buffer();
		bufferInfos[3].offset = 0;
		bufferInfos[3].range = m_drawDataBuffer->get_size();

		//X Material Data
		bufferInfos[4].buffer = m_materialBuffer->get_vk_buffer();
		bufferInfos[4].offset = 0;
		bufferInfos[4].range = m_materialBuffer->get_size();

		//X Transform Data
		bufferInfos[5].buffer = m_transformBuffer->get_vk_buffer();
		bufferInfos[5].offset = 0;
		bufferInfos[5].range = m_transformBuffer->get_size();

		//X Mesh Data
		bufferInfos[6].buffer = m_meshBuffer->get_vk_buffer();
		bufferInfos[6].offset = 0;
		bufferInfos[6].range = m_meshBuffer->get_size();

		std::array< VkWriteDescriptorSet, 7> setWrites;
		setWrites[0] = {};
		setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[0].pNext = nullptr;
		setWrites[0].dstBinding = 0;
		setWrites[0].dstSet = vkdescriptorSets[i];
		setWrites[0].descriptorCount = 1;
		setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setWrites[0].pBufferInfo = &(bufferInfos[0]);
		//-------------------
		setWrites[1] = {};
		setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[1].pNext = nullptr;
		setWrites[1].dstBinding = 1;
		setWrites[1].dstSet = vkdescriptorSets[i];
		setWrites[1].descriptorCount = 1;
		setWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[1].pBufferInfo = &(bufferInfos[1]);
		
		setWrites[2] = {};
		setWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[2].pNext = nullptr;
		setWrites[2].dstBinding = 2;
		setWrites[2].dstSet = vkdescriptorSets[i];
		setWrites[2].descriptorCount = 1;
		setWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[2].pBufferInfo = bufferInfos.data() + 2;

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

		//X Mesh Data

		setWrites[6] = {};
		setWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[6].pNext = nullptr;
		setWrites[6].dstBinding = 6;
		setWrites[6].dstSet = vkdescriptorSets[i];
		setWrites[6].descriptorCount = 1;
		setWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[6].pBufferInfo = bufferInfos.data() + 6;

		vkUpdateDescriptorSets(m_boundedDevice->get_vk_device(), setWrites.size(), setWrites.data(), 0, nullptr);

	}

	(*descriptorSets) = vkdescriptorSets;
	return pool;

}
