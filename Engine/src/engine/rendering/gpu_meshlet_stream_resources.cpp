#include "internal/engine/rendering/gpu_meshlet_stream_resources.h"
#include <array>

#include <unordered_map>
#include <cassert>
#include "engine/rendering/vulkan/ivulkan_descriptor_pool.h"
#include "engine/gengine.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/rendering/vulkan/ivulkan_app.h"


GPUMeshletStreamResources::GPUMeshletStreamResources(IGVulkanLogicalDevice* boundedDevice, IGPipelineObjectManager* pipelineManager, uint32_t frameInFlight)
{
	m_framesInFlight = frameInFlight;
	p_boundedDevice = boundedDevice;
	p_pipelineManager = pipelineManager;
}

bool GPUMeshletStreamResources::init(uint32_t beginMeshCount, uint32_t beginGMeshletCount, uint32_t beginMeshletIndicesCount, uint32_t beginPrimitiveIndicesCount)
{
	vkCmdDrawMeshTasksIndirectCountEXTMethod = (PFN_vkCmdDrawMeshTasksIndirectCountEXT)vkGetInstanceProcAddr((VkInstance)p_boundedDevice->get_bounded_physical_device()->get_bounded_app()->get_vk_instance(), "vkCmdDrawMeshTasksIndirectCountEXT");
	if (vkCmdDrawMeshTasksIndirectCountEXTMethod == nullptr)
		return false;

	m_meshletExtraData.gpuBuffer.reset(p_boundedDevice->create_buffer(beginMeshCount * sizeof(GMeshletExtra), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_meshletExtraData.create_internals();

	m_mergedGMeshlet.gpuBuffer.reset(p_boundedDevice->create_buffer(beginGMeshletCount * sizeof(GMeshlet), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_mergedGMeshlet.create_internals();

	m_mergedMeshletTriangles.gpuBuffer.reset(p_boundedDevice->create_buffer(beginPrimitiveIndicesCount * sizeof(uint8_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_mergedMeshletTriangles.create_internals();


	m_mergedMeshletVertex.gpuBuffer.reset(p_boundedDevice->create_buffer(beginMeshletIndicesCount * sizeof(uint32_t), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_mergedMeshletVertex.create_internals();

	//X Init MeshletExtraLayout
	{
		//X GMeshletExtra
		std::array<VkDescriptorSetLayoutBinding, 4> bindings;
		bindings[0].binding = 0;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
		bindings[0].pImmutableSamplers = nullptr;

		//X GMeshMeshlet
		bindings[1].binding = 1;
		bindings[1].descriptorCount = 1;
		bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
		bindings[1].pImmutableSamplers = nullptr;

		//X GMeshletVertexData
		bindings[2].binding = 2;
		bindings[2].descriptorCount = 1;
		bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
		bindings[2].pImmutableSamplers = nullptr;

		//X GMeshMeshletTriangleData
		bindings[3].binding = 3;
		bindings[3].descriptorCount = 1;
		bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		bindings[3].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;
		bindings[3].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo setinfo = {};
		setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setinfo.pNext = nullptr;
		setinfo.bindingCount = bindings.size();
		setinfo.flags = 0;
		setinfo.pBindings = bindings.data();

		m_meshletInfoSetLayout = p_pipelineManager->create_or_get_named_set_layout("MeshletInfoStreamSetLayout", &setinfo);
	}

	std::unordered_map<VkDescriptorType, int> types;
	types.emplace(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4);
	m_generalPool = p_boundedDevice->create_and_init_vector_pool(types, m_framesInFlight);
	assert(m_generalPool != nullptr);

	//X Create Descriptor Sets
	{
		std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight, m_meshletInfoSetLayout->get_layout());

		//allocate one descriptor set for each frame
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_generalPool->get_vk_descriptor_pool();
		allocInfo.descriptorSetCount = layouts.size();
		allocInfo.pSetLayouts = layouts.data();

		m_meshletInfoSets.resize(m_framesInFlight);
		auto setAllocRes = vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &allocInfo, m_meshletInfoSets.data());
		assert(VK_SUCCESS == setAllocRes);
	}
	//X Write Sets
	{
		std::array<VkDescriptorBufferInfo, 4> bufferInfos;
		//X MeshExtra
		bufferInfos[0].buffer = m_meshletExtraData.gpuBuffer->get_vk_buffer();
		bufferInfos[0].offset = 0;
		bufferInfos[0].range = m_meshletExtraData.gpuBuffer->get_size();

		bufferInfos[1].buffer = m_mergedGMeshlet.gpuBuffer->get_vk_buffer();
		bufferInfos[1].offset = 0;
		bufferInfos[1].range = m_mergedGMeshlet.gpuBuffer->get_size();

		bufferInfos[2].buffer = m_mergedMeshletVertex.gpuBuffer->get_vk_buffer();
		bufferInfos[2].offset = 0;
		bufferInfos[2].range = m_mergedMeshletVertex.gpuBuffer->get_size();

		bufferInfos[3].buffer = m_mergedMeshletTriangles.gpuBuffer->get_vk_buffer();
		bufferInfos[3].offset = 0;
		bufferInfos[3].range = m_mergedMeshletTriangles.gpuBuffer->get_size();

		std::array< VkWriteDescriptorSet, 4> setWrites;
		setWrites[0] = {};
		setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[0].pNext = nullptr;
		setWrites[0].dstBinding = 0;
		setWrites[0].descriptorCount = 1;
		setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
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

		setWrites[3] = {};
		setWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[3].pNext = nullptr;
		setWrites[3].dstBinding = 3;
		setWrites[3].descriptorCount = 1;
		setWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[3].pBufferInfo = bufferInfos.data() + 3;


		for (int i = 0; i < m_framesInFlight; i++)
		{
			setWrites[0].dstSet = m_meshletInfoSets[i];
			setWrites[1].dstSet = m_meshletInfoSets[i];
			setWrites[2].dstSet = m_meshletInfoSets[i];
			setWrites[3].dstSet = m_meshletInfoSets[i];

			vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), setWrites.size(), setWrites.data(), 0, nullptr);

		}
	}
	return true;
}

void GPUMeshletStreamResources::destroy()
{
	m_mergedGMeshlet.destroy();
	m_mergedMeshletVertex.destroy();
	m_mergedMeshletTriangles.destroy();
	m_meshletExtraData.destroy();
}

IGVulkanNamedSetLayout* GPUMeshletStreamResources::get_meshlet_set_layout() const noexcept
{
	return m_meshletInfoSetLayout;
}

VkDescriptorSet GPUMeshletStreamResources::get_set_by_index(uint32_t index) const
{
	return m_meshletInfoSets[index];
}

void GPUMeshletStreamResources::cmd_draw_indirect_data(GVulkanCommandBuffer* cmd, uint32_t frame,uint32_t maxIndirectDrawCommand,IVulkanBuffer* indirectBuffer,uint32_t countOffset)
{
	vkCmdDrawMeshTasksIndirectCountEXTMethod(cmd->get_handle(), indirectBuffer->get_vk_buffer(), countOffset,
		indirectBuffer->get_vk_buffer(), 0, maxIndirectDrawCommand, sizeof(VkDrawMeshTasksIndirectCommandEXT));
}

uint32_t GPUMeshletStreamResources::add_meshlet_data(const GMeshletData* meshlet)
{
	//X Add meshket bounded data
	uint32_t beginGMeshlet = m_mergedGMeshlet.add_to_buffer(meshlet->gmeshlets_);
	uint32_t beginMeshletVertex = m_mergedMeshletVertex.add_to_buffer(meshlet->meshletVertexData_);
	uint32_t beginMeshletTriangle = m_mergedMeshletTriangles.add_to_buffer(meshlet->meshletTriangleData_);
	//X Create gmesh data for per mesh
	std::vector<GMeshletExtra> gmeshExtra(meshlet->gmeshMeshlets_.size());
	for (int i = 0; i < gmeshExtra.size(); i++)
	{
		auto& gmeshExtraData = gmeshExtra[i];

		gmeshExtraData.meshletTrianglesOffset = beginMeshletTriangle + meshlet->gmeshMeshlets_[i].meshletTrianglesOffset;
		gmeshExtraData.meshletVerticesOffset = beginMeshletVertex + meshlet->gmeshMeshlets_[i].meshletVerticesOffset;
		gmeshExtraData.meshletOffset = beginGMeshlet + meshlet->gmeshMeshlets_[i].meshletOffset;

		gmeshExtraData.meshletTrianglesCount = meshlet->gmeshMeshlets_[i].meshletTrianglesCount;
		gmeshExtraData.meshletVerticesCount = meshlet->gmeshMeshlets_[i].meshletVerticesCount;
		gmeshExtraData.meshletCount = meshlet->gmeshMeshlets_[i].meshletCount;
	}

	return m_meshletExtraData.add_to_buffer(gmeshExtra);
}

uint32_t GPUMeshletStreamResources::add_meshlet_data(const GMeshletDataExtra* gmeshlet)
{
	uint32_t beginGMeshlet = m_mergedGMeshlet.add_to_buffer(gmeshlet->gmeshlets_);
	uint32_t beginMeshletVertex = m_mergedMeshletVertex.add_to_buffer(gmeshlet->meshletVertexData_);
	uint32_t beginMeshletTriangle = m_mergedMeshletTriangles.add_to_buffer(gmeshlet->meshletTriangleData_);
	//X Create gmesh data for per mesh
	//X TODO : DONT COPY
	std::vector<GMeshletExtra> gmeshExtra(gmeshlet->gmeshletExtra_.size());
	for (int i = 0; i < gmeshExtra.size(); i++)
	{
		auto& gmeshExtraData = gmeshExtra[i];

		gmeshExtraData.meshletTrianglesOffset = beginMeshletTriangle;
		gmeshExtraData.meshletVerticesOffset = beginMeshletVertex;
		gmeshExtraData.meshletOffset = beginGMeshlet + gmeshlet->gmeshletExtra_[i].meshletOffset;

		gmeshExtraData.meshletTrianglesCount = gmeshlet->gmeshletExtra_[i].meshletTrianglesCount;
		gmeshExtraData.meshletVerticesCount = gmeshlet->gmeshletExtra_[i].meshletVerticesCount;
		gmeshExtraData.meshletCount = gmeshlet->gmeshletExtra_[i].meshletCount;
	}

	return m_meshletExtraData.add_to_buffer(gmeshExtra);
}
