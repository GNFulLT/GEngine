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
	m_computeSetLayout = nullptr;
	m_inUsageSizeGlobalDrawDataBuffer = 0;
}

bool GPUMeshStreamResources::init(uint32_t beginVertexCount, uint32_t beginIndexCount, uint32_t beginMeshCount, uint32_t beginDrawDataAndIdCount)
{
	m_mergedVertex.gpuBuffer.reset(p_boundedDevice->create_buffer(uint64_t(beginVertexCount) * m_floatPerVertex * sizeof(float), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_mergedVertex.create_internals();

	m_mergedIndex.gpuBuffer.reset(p_boundedDevice->create_buffer(beginIndexCount * sizeof(uint32_t), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_mergedIndex.create_internals();
	
	m_mergedMesh.gpuBuffer.reset(p_boundedDevice->create_buffer(beginMeshCount * sizeof(GMeshData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_mergedMesh.create_internals();

	m_globalDrawData.gpuBuffer.reset(p_boundedDevice->create_buffer(beginDrawDataAndIdCount * sizeof(DrawData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_globalDrawData.create_internals();

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
		m_globalIndirectCommandBuffers[i].reset(p_boundedDevice->create_buffer(uint64_t(m_globalIndirectCommandsBeginOffset) + sizeOfIndirectCommands, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY).value());
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
		types.emplace(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,8);
		m_generalPool = p_boundedDevice->create_and_init_vector_pool(types,m_framesInFlight*2);
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
			auto setAllocRes = vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &allocInfo, m_drawStreamSets.data());
			assert(VK_SUCCESS == setAllocRes);

		}

	}
	return true;
}

void GPUMeshStreamResources::add_mesh_data(MeshData* meshData)
{
	//X Add the vertices and indices
	uint32_t beginVertexFloat = m_mergedVertex.add_to_buffer(meshData->vertexData_);
	uint32_t beginIndex = m_mergedIndex.add_to_buffer(meshData->indexData_);
	//X Create gmesh data for per mesh
	std::vector<GMeshData> gmeshes(meshData->meshes_.size());
	for (int i = 0; i < meshData->meshes_.size(); i++)
	{
		GMeshData* gmesh = &gmeshes[i];
		gmesh->boundingBox = meshData->boxes_[i];
		gmesh->extent = glm::vec4(0,0,0,0);
		gmesh->vertexOffset += (beginVertexFloat/ this->m_floatPerVertex) + meshData->meshes_[i].vertexOffset;
		gmesh->indexOffset += beginIndex + meshData->meshes_[i].indexOffset; 
		gmesh->vertexCount = meshData->meshes_[i].vertexCount;
		gmesh->lodCount = meshData->meshes_[i].lodCount;
		memcpy(&gmeshes[i].lodOffset[0], &meshData->meshes_[i].lodOffset[0], sizeof(uint32_t) * MeshConstants::MAX_LOD_COUNT);
	}
	uint32_t beginMeshIndex = m_mergedMesh.add_to_buffer(gmeshes);
}

void GPUMeshStreamResources::destroy()
{
	//X Destroy Pool
	if (m_generalPool != nullptr)
	{
		m_generalPool->destroy();
		delete m_generalPool;
		m_generalPool = nullptr;

		m_computeSets.clear();
		m_drawStreamSets.clear();
	}
	//X Destroy Layouts 
	{
		if (m_computeSetLayout != nullptr)
		{
			m_computeSetLayout->destroy();
			m_computeSetLayout = nullptr;
		}
		if (m_drawStreamSetLayout!= nullptr)
		{
			m_drawStreamSetLayout->destroy();
			m_drawStreamSetLayout = nullptr;
		}
	}

	//X Destroy the buffers
	{
		//X Global Indirect Buffer
		for (auto& bf : m_globalIndirectCommandBuffers)
		{
			bf->unload();
			bf.reset();
		}
		//X  Draw IDs
		m_globalDrawIdBuffer->unload();
		m_globalDrawIdBuffer.reset();

		//X CPUGPU Buffers
		m_globalDrawData.destroy();
		m_mergedMesh.destroy();
		m_mergedVertex.destroy();
		m_mergedIndex.destroy();
	}

}

uint32_t GPUMeshStreamResources::get_count_of_draw_data()
{
	return m_globalDrawData.cpuVector.size();
}

IGVulkanNamedSetLayout* GPUMeshStreamResources::get_draw_set_layout()
{
	return m_drawStreamSetLayout;
}

IGVulkanNamedSetLayout* GPUMeshStreamResources::get_compute_set_layout()
{
	return m_computeSetLayout;
}

VkDescriptorSet_T* GPUMeshStreamResources::get_draw_set_by_index(uint32_t currentFrame)
{
	return m_drawStreamSets[currentFrame];
}

VkDescriptorSet_T* GPUMeshStreamResources::get_compute_set_by_index(uint32_t currentFrame)
{
	return m_computeSets[currentFrame];
}

void GPUMeshStreamResources::update_draw_data_sets()
{
	std::array<VkDescriptorBufferInfo, 3> bufferInfos;
	//X Mesh
	bufferInfos[0].buffer = m_mergedMesh.gpuBuffer->get_vk_buffer();
	bufferInfos[0].offset = 0;
	bufferInfos[0].range = m_mergedMesh.gpuBuffer->get_size();
	//X DrawData
	bufferInfos[1].buffer = m_globalDrawData.gpuBuffer->get_vk_buffer();
	bufferInfos[1].offset = 0;
	bufferInfos[1].range = m_globalDrawData.gpuBuffer->get_size();
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

