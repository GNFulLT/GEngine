#include "volk.h"
#include "internal/engine/rendering/gpu_mesh_stream_resources.h"
#include "vma/vk_mem_alloc.h"
#include "engine/rendering/vulkan/ivulkan_pdevice.h"
#include <array>
#include <unordered_map>
#include <cassert>
#include "engine/rendering/vulkan/ivulkan_descriptor_pool.h"
GPUMeshStreamResources::GPUMeshStreamResources(IGVulkanLogicalDevice* dev,uint32_t floatCountPerVertex, uint32_t framesInFlight, IGPipelineObjectManager* mng)
{
	m_drawStreamSetLayout = nullptr;
	p_pipelineObjectMng = mng;
	m_framesInFlight = framesInFlight;
	m_floatPerVertex = floatCountPerVertex;
	p_boundedDevice = dev;

	m_inUsageSizeGlobalDrawDataBuffer = 0;
	m_inUsageSizeIndexBuffer = 0;
	m_inUsageSizeMeshBuffer = 0;
	m_inUsageSizeVertexSize = 0;

	m_mergedIndexBufferMappedMem = nullptr;
	m_mergedMeshBufferMappedMem = nullptr;
	m_mergedVertexBufferMappedMem = nullptr;
	m_globalDrawDataBufferMappedMem = nullptr;
}

bool GPUMeshStreamResources::init(uint32_t beginVertexCount, uint32_t beginIndexCount, uint32_t beginMeshCount, uint32_t beginDrawDataAndIdCount)
{
	m_mergedVertexBuffer.reset(p_boundedDevice->create_buffer(uint64_t(beginVertexCount) * m_floatPerVertex * sizeof(float), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_mergedVertexBufferMappedMem = m_mergedVertexBuffer->map_memory();

	m_mergedIndexBuffer.reset(p_boundedDevice->create_buffer(beginIndexCount * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_mergedIndexBufferMappedMem = (uint32_t*)m_mergedIndexBuffer->map_memory();
	
	m_mergedMeshBuffer.reset(p_boundedDevice->create_buffer(beginMeshCount * sizeof(GMeshData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_mergedMeshBufferMappedMem = (GMeshData*)m_mergedMeshBuffer->map_memory();
	
	m_globalDrawDataBuffer.reset(p_boundedDevice->create_buffer(beginDrawDataAndIdCount * sizeof(DrawData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_globalDrawDataBufferMappedMem = (DrawData*)m_globalDrawDataBuffer->map_memory();

	m_globalDrawIdBuffer.reset(p_boundedDevice->create_buffer(beginDrawDataAndIdCount * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY).value());
	
	//X Calculate the indirect buffer memory
	uint32_t sizeOfIndirectCommands = beginDrawDataAndIdCount * sizeof(VkDrawIndexedIndirectCommand);
	
	m_globalIndirectCommandsBeginOffset = sizeof(uint32_t);
	//X Fill the offset 
	auto minStorageAlignment = p_boundedDevice->get_bounded_physical_device()->get_vk_properties()->limits.minStorageBufferOffsetAlignment;
	if ((m_globalIndirectCommandsBeginOffset & (minStorageAlignment - 1)) != 0)
	{
		m_globalIndirectCommandsBeginOffset = (m_globalIndirectCommandsBeginOffset + minStorageAlignment) & ~(minStorageAlignment - 1);
	}

	m_globalIndirectCommandBuffers.resize(m_framesInFlight);
	for (int i = 0; i < m_framesInFlight; i++)
	{
		m_globalIndirectCommandBuffers[i].reset(p_boundedDevice->create_buffer(uint64_t(m_globalIndirectCommandsBeginOffset) + sizeOfIndirectCommands, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY).value());
	}

	//X Create named sets

	//X First create the DrawStreamSet
	{
		//X Mesh Buffer
		std::array<VkDescriptorSetLayoutBinding, 3> bindings;
		bindings[0].binding = 0;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[0].pImmutableSamplers = nullptr;

		//X DrawData Buffer
		bindings[1].binding = 1;
		bindings[1].descriptorCount = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[1].pImmutableSamplers = nullptr;

		//X DrawID Buffer
		bindings[2].binding = 2;
		bindings[2].descriptorCount = 1;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		bindings[2].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo setinfo = {};
		setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setinfo.pNext = nullptr;
		setinfo.bindingCount = bindings.size();
		setinfo.flags = 0;
		setinfo.pBindings = bindings.data();

		m_drawStreamSetLayout = p_pipelineObjectMng->create_or_get_named_set_layout("DrawStreamSet", &setinfo);
	}
	//X Compute Set
	{		
		//X Indirect Count Buffer
		std::array<VkDescriptorSetLayoutBinding, 2> bindings;
		bindings[0].binding = 0;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[0].pImmutableSamplers = nullptr;

		//X Indirect Draw Buffer
		bindings[1].binding = 1;
		bindings[1].descriptorCount = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[1].pImmutableSamplers = nullptr;
		VkDescriptorSetLayoutCreateInfo setinfo = {};
		setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setinfo.pNext = nullptr;
		setinfo.bindingCount = bindings.size();
		setinfo.flags = 0;
		setinfo.pBindings = bindings.data();

		m_computeSetLayout = p_pipelineObjectMng->create_or_get_named_set_layout("ComputeSet", &setinfo);
	}

	//X Allocate sets
	{
		std::unordered_map<VkDescriptorType, int> types;
		types.emplace(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,5);
		m_generalPool = p_boundedDevice->create_and_init_vector_pool(types);
		assert(m_generalPool != nullptr);
		//X Compute Set
		{
			std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight, m_computeSetLayout->get_layout());

			//allocate one descriptor set for each frame
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.pNext = nullptr;
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_generalPool->get_vk_descriptor_pool();
			allocInfo.descriptorSetCount = layouts.size();
			allocInfo.pSetLayouts = layouts.data();

			m_computeSets.resize(m_framesInFlight);

			assert(VK_SUCCESS == vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &allocInfo, m_computeSets.data()));
		}
		//X DrawStreamSet
		{
			std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight, m_drawStreamSetLayout->get_layout());

			//allocate one descriptor set for each frame
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.pNext = nullptr;
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_generalPool->get_vk_descriptor_pool();
			allocInfo.descriptorSetCount = layouts.size();
			allocInfo.pSetLayouts = layouts.data();

			m_drawStreamSets.resize(m_framesInFlight);

			assert(VK_SUCCESS == vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &allocInfo, m_drawStreamSets.data()));

		}

	}
}

void GPUMeshStreamResources::update_draw_data_sets()
{
	std::array<VkDescriptorBufferInfo, 3> bufferInfos;
	//X Mesh
	bufferInfos[0].buffer = m_mergedMeshBuffer->get_vk_buffer();
	bufferInfos[0].offset = 0;
	bufferInfos[0].range = m_mergedMeshBuffer->get_size();
	//X DrawData
	bufferInfos[1].buffer = m_globalDrawDataBuffer->get_vk_buffer();
	bufferInfos[1].offset = 0;
	bufferInfos[1].range = m_globalDrawDataBuffer->get_size();
	//X DrawID
	bufferInfos[2].buffer = m_globalDrawIdBuffer->get_vk_buffer();
	bufferInfos[2].offset = 0;
	bufferInfos[2].range = m_globalDrawIdBuffer->get_size();

	std::array< VkWriteDescriptorSet, 3> setWrites;
	setWrites[0] = {};
	setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWrites[0].pNext = nullptr;
	setWrites[0].dstBinding = 0;
	setWrites[0].descriptorCount = 1;
	setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	setWrites[0].pBufferInfo = &(bufferInfos[0]);
	//-------------------
	setWrites[1] = {};
	setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWrites[1].pNext = nullptr;
	setWrites[1].dstBinding = 1;
	setWrites[1].descriptorCount = 1;
	setWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	setWrites[1].pBufferInfo = &(bufferInfos[1]);

	setWrites[2] = {};
	setWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWrites[2].pNext = nullptr;
	setWrites[2].dstBinding = 2;
	setWrites[2].descriptorCount = 1;
	setWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	setWrites[2].pBufferInfo = bufferInfos.data() + 2;


	for (int i = 0; i < m_framesInFlight; i++)
	{
		setWrites[0].dstSet = m_drawStreamSets[i];
		setWrites[1].dstSet = m_drawStreamSets[i];
		setWrites[2].dstSet = m_drawStreamSets[i];

		vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), setWrites.size(), setWrites.data(), 0, nullptr);
	}
}

void GPUMeshStreamResources::update_compute_sets()
{
	std::array<VkDescriptorBufferInfo, 2> bufferInfos;
	std::array< VkWriteDescriptorSet, 2> setWrites;
	setWrites[0] = {};
	setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWrites[0].pNext = nullptr;
	setWrites[0].dstBinding = 0;
	setWrites[0].descriptorCount = 1;
	setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	setWrites[0].pBufferInfo = &(bufferInfos[0]);
	//-------------------
	setWrites[1] = {};
	setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWrites[1].pNext = nullptr;
	setWrites[1].dstBinding = 1;
	setWrites[1].descriptorCount = 1;
	setWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	setWrites[1].pBufferInfo = &(bufferInfos[1]);

	for (int i = 0; i < m_framesInFlight; i++)
	{
		//X Count Buffer
		bufferInfos[0].buffer = m_globalIndirectCommandBuffers[i]->get_vk_buffer();
		bufferInfos[0].offset = 0;
		bufferInfos[0].range = m_globalIndirectCommandsBeginOffset;
		//X Indirect
		bufferInfos[1].buffer = m_globalIndirectCommandBuffers[i]->get_vk_buffer();
		bufferInfos[1].offset = m_globalIndirectCommandsBeginOffset;
		bufferInfos[1].range = m_globalIndirectCommandBuffers[i]->get_size() - m_globalIndirectCommandsBeginOffset;


		setWrites[0].dstSet = m_computeSets[i];
		setWrites[1].dstSet = m_computeSets[i];

		vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), setWrites.size(), setWrites.data(), 0, nullptr);
	}
	
}

