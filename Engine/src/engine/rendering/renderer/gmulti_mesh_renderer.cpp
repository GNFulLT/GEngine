#include "volk.h"
#include "internal/engine/rendering/renderer/gmulti_mesh_renderer.h"
#include <cassert>
#include <array>
#include "public/math/gmat4.h"
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"
#include "engine/rendering/vulkan/ivulkan_pdevice.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "public/core/templates/shared_ptr.h"
#include "engine/resource/igshader_resource.h"

GMultiMeshRenderer::GMultiMeshRenderer(IGVulkanLogicalDevice* dev, IGCameraManager* cam, IGResourceManager* res,
	IGShaderManager* sm,uint32_t frameCount, Scene* scene, std::vector<MaterialDescription>& materials, MeshData* meshData)
{
	m_scene = scene;
	m_materialDatas = materials;
	m_meshDatas = meshData;
	m_wireframeSpec.thickness = 0.5f;
	m_wireframeSpec.step = 4.0f;

	m_res = res;
	m_sm = sm;
	m_cameraManager = cam;
	m_boundedDevice = dev;
	m_frameCount = frameCount;
	maxVertexBufferSize_ = m_meshDatas->vertexData_.size() * sizeof(float);
	//X Pad the offset if it needs
	auto minStorageAlignment = m_boundedDevice->get_bounded_physical_device()->get_vk_properties()->limits.minStorageBufferOffsetAlignment;
	if ((maxVertexBufferSize_ & (minStorageAlignment - 1)) != 0)
	{
		maxVertexBufferSize_ = (maxVertexBufferSize_ + minStorageAlignment) & ~(minStorageAlignment - 1);
	}
	
	maxMaterialSize_ = m_materialDatas.size() * sizeof(MaterialDescription);

	maxIndexBufferSize_ = m_meshDatas->indexData_.size() * sizeof(uint32_t);
	
	maxInstances_ = m_meshDatas->meshes_.size();
	
	maxInstanceSize_ = maxInstances_ * sizeof(InstanceData);

	m_instanceDatas.resize(maxInstances_);
	

	//X It only works for 
	for (int i = 0; i < m_instanceDatas.size(); i++)
	{
		m_instanceDatas[i] = {};
		m_instanceDatas[i].lod = 0;
		m_instanceDatas[i].indexOffset = m_meshDatas->meshes_[i].indexOffset + (m_meshDatas->meshes_[i].lodOffset[m_instanceDatas[i].lod] - m_meshDatas->meshes_[i].lodOffset[0]);
		//X Find the corresponded material 
		int materialIndex = 0;
		int transformIndex = 0;
		for (int j = 0; j < m_scene->hierarchy.size(); j++)
		{
			if (auto msh =m_scene->meshes_.find(j);msh != m_scene->meshes_.end())
			{
				if (i == msh->second)
				{
					if (auto mtrl = m_scene->materialForNode_.find(j);mtrl != m_scene->materialForNode_.end())
					{
						materialIndex = mtrl->second;
					}
					transformIndex = j;
				}
			}	
		}
		m_instanceDatas[i].material = materialIndex;
		m_instanceDatas[i].mesh = i;
		m_instanceDatas[i].transformIndex = transformIndex;
		m_instanceDatas[i].vertexOffset = m_meshDatas->meshes_[i].vertexOffset;
	}

}

bool GMultiMeshRenderer::init(IGVulkanViewport* port)
{
	m_vert = m_res->create_shader_resource("instanced", "instancedGroup", "./indirect_draw.glsl_vert").value();
	assert(m_vert->load() == RESOURCE_INIT_CODE_OK);
	
	m_frag = m_res->create_shader_resource("instanced", "instancedGroup", "./wireframe.glsl_frag").value();
	assert(m_frag->load() == RESOURCE_INIT_CODE_OK);

	m_comp = m_res->create_shader_resource("compute", "instancedGroup", "./frustum_cull.glsl_comp").value();
	assert(m_comp->load() == RESOURCE_INIT_CODE_OK);

	

	//X Create storage buffers
	//X Index and vertex buffer will be stored in here
	m_storageBuffer.reset(m_boundedDevice->create_storage_buffer(maxVertexBufferSize_ + maxIndexBufferSize_).value());
	//X Upload the vertices and indices
	void* storageData = m_storageBuffer->map_memory();
	void* indexBegin = (void*)(std::size_t(storageData) + maxVertexBufferSize_);
	memcpy(storageData, m_meshDatas->vertexData_.data(), maxVertexBufferSize_);
	memcpy(indexBegin, m_meshDatas->indexData_.data(), maxIndexBufferSize_);
	m_storageBuffer->unmap_memory();
	
	m_instancedBuffers.reset(m_boundedDevice->create_storage_buffer(m_instanceDatas.size() * sizeof(InstanceData)).value());

	//X Upload instance datas
	m_instancedBuffers->copy_data_to_device_memory(m_instanceDatas.data(), m_instanceDatas.size()*sizeof(InstanceData));

	const uint32_t indirectDataSize = maxInstances_ * sizeof(VkDrawIndirectCommand);
	for (int i = 0; i < m_frameCount; i++)
	{
		m_indirectBuffers.push_back(m_boundedDevice->create_indirect_buffer(indirectDataSize).value());
		update_indirect_commands(i);
	}

	//X Create buffer for transforms

	//m_transformBuffer.reset(m_boundedDevice->create_storage_buffer(m_scene->globalTransform_.size() * sizeof(glm::mat4)).value());

	//m_scene->globalTransform_[0] = glm::mat4(1.f) * glm::scale(glm::mat4(1.f),glm::vec3(50.f,50.f,50.f)) * glm::translate(glm::mat4(1.f), glm::vec3(0,0, 0));
	//m_transformBuffer->copy_data_to_device_memory(m_scene->globalTransform_.data(), m_scene->globalTransform_.size() * sizeof(glm::mat4));

	//X Create mesh buffer for mesh datas
	m_meshDataBuffer.reset(m_boundedDevice->create_storage_buffer(m_meshDatas->meshes_.size()*sizeof(MeshData)).value());
	m_meshDataBuffer->copy_data_to_device_memory(m_meshDatas->meshes_.data(), m_meshDatas->meshes_.size() * sizeof(MeshData));

	//X Create material buffer
	m_materialBuffer.reset(m_boundedDevice->create_storage_buffer(m_materialDatas.size()*sizeof(MaterialDescription)).value());
	m_materialBuffer->copy_data_to_device_memory(m_materialDatas.data(), m_materialBuffer->get_size());

	//X Create bounding box buffer
	auto boxSize =sizeof(BoundingBox);
	m_boundingBoxBuffer.reset(m_boundedDevice->create_storage_buffer(m_meshDatas->boxes_.size()*boxSize).value());
	m_boundingBoxBuffer->copy_data_to_device_memory(m_meshDatas->boxes_.data(),boxSize*m_meshDatas->boxes_.size());
	m_cullDataBuffer.reset(m_boundedDevice->create_uniform_buffer(sizeof(DrawCullData)).value());
	m_cullDataBufferMappedMem = m_cullDataBuffer->map_memory();
	//X Create layouts
	auto sceneMng = ((GSharedPtr<IGSceneManager>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCENE))->get();

	m_layout = new GMultiMeshRendererLayout(m_boundedDevice, sceneMng, m_storageBuffer.get(), m_meshDataBuffer.get(), m_instancedBuffers.get(), m_transformBuffer.get(), m_materialBuffer.get(), maxVertexBufferSize_, maxIndexBufferSize_, m_frameCount);

	//X Now create shaders and stage states
	m_vertStage	 = m_sm->create_shader_stage_from_shader_res(m_vert).value();
	m_fragStage = m_sm->create_shader_stage_from_shader_res(m_frag).value();
	m_compStage = m_sm->create_shader_stage_from_shader_res(m_comp).value();

	// ------------------------- Comp Creation


	//X Create pipeline layout for compute pipeline
	std::unordered_map<VkDescriptorType, int> poolObjs;
	poolObjs.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
	poolObjs.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 5);

	m_compPool = m_boundedDevice->create_and_init_vector_pool(poolObjs, m_frameCount);
	std::array<VkDescriptorSetLayoutBinding, 7> bindings;
	//X Cam Buff
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	//X VERTEX BUFF
	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[1].pImmutableSamplers = nullptr;

	//X INDEX BUFF
	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[2].pImmutableSamplers = nullptr;

	//X Draw Data
	bindings[3].binding = 3;
	bindings[3].descriptorCount = 1;
	bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[3].pImmutableSamplers = nullptr;

	//X Cull Data
	bindings[4].binding = 4;
	bindings[4].descriptorCount = 1;
	bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[4].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[4].pImmutableSamplers = nullptr;

	//X Mesh Data
	bindings[5].binding = 5;
	bindings[5].descriptorCount = 1;
	bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[5].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[5].pImmutableSamplers = nullptr;

	//X Draw Instance Data
	bindings[6].binding = 6;
	bindings[6].descriptorCount = 1;
	bindings[6].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[6].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
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

	auto res = vkCreateDescriptorSetLayout(m_boundedDevice->get_vk_device(), &setinfo, nullptr, &m_compSetLayout);
	assert(res == VK_SUCCESS);

	std::vector<VkDescriptorSetLayout> layouts(m_frameCount, m_compSetLayout);

	//allocate one descriptor set for each frame
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_compPool->get_vk_descriptor_pool();
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	m_compSets.resize(m_frameCount);

	assert(VK_SUCCESS == vkAllocateDescriptorSets(m_boundedDevice->get_vk_device(), &allocInfo, m_compSets.data()));

	//X Update sets
	for (int i = 0; i < m_compSets.size(); i++)
	{
		auto gbuff = sceneMng->get_global_buffer_for_frame(i);
		std::array<VkDescriptorBufferInfo, 7> bufferInfos;
		//it will be the camera buffer
		bufferInfos[0].buffer = gbuff->get_vk_buffer();
		//at 0 offset
		bufferInfos[0].offset = 0;
		//of the size of a camera data struct
		bufferInfos[0].range = gbuff->get_size();
		//X Draw Data
		bufferInfos[1].buffer = m_indirectBuffers[i]->get_vk_buffer();
		bufferInfos[1].offset = 0;
		bufferInfos[1].range = m_indirectBuffers[i]->get_size();

		//X Transform Data
		bufferInfos[2].buffer = m_transformBuffer->get_vk_buffer();
		bufferInfos[2].offset = 0;
		bufferInfos[2].range = m_transformBuffer->get_size();

		//X Draw Data
		bufferInfos[3].buffer = m_boundingBoxBuffer->get_vk_buffer();
		bufferInfos[3].offset = 0;
		bufferInfos[3].range = m_boundingBoxBuffer->get_size();

		//X Cull Data
		bufferInfos[4].buffer = m_cullDataBuffer->get_vk_buffer();
		bufferInfos[4].offset = 0;
		bufferInfos[4].range = m_cullDataBuffer->get_size();

		//X DrawInstance Data
		bufferInfos[5].buffer = m_instancedBuffers->get_vk_buffer();
		bufferInfos[5].offset = 0;
		bufferInfos[5].range = m_instancedBuffers->get_size();

		//X Mesh Data
		bufferInfos[6].buffer = m_meshDataBuffer->get_vk_buffer();
		bufferInfos[6].offset = 0;
		bufferInfos[6].range = m_meshDataBuffer->get_size();

		std::array< VkWriteDescriptorSet, 7> setWrites;
		setWrites[0] = {};
		setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[0].pNext = nullptr;
		setWrites[0].dstBinding = 0;
		setWrites[0].dstSet = m_compSets[i];
		setWrites[0].descriptorCount = 1;
		setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setWrites[0].pBufferInfo = &(bufferInfos[0]);
		//-------------------
		setWrites[1] = {};
		setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[1].pNext = nullptr;
		setWrites[1].dstBinding = 1;
		setWrites[1].dstSet = m_compSets[i];
		setWrites[1].descriptorCount = 1;
		setWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[1].pBufferInfo = &(bufferInfos[1]);

		setWrites[2] = {};
		setWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[2].pNext = nullptr;
		setWrites[2].dstBinding = 2;
		setWrites[2].dstSet = m_compSets[i];
		setWrites[2].descriptorCount = 1;
		setWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[2].pBufferInfo = bufferInfos.data() + 2;

		setWrites[3] = {};
		setWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[3].pNext = nullptr;
		setWrites[3].dstBinding = 3;
		setWrites[3].dstSet = m_compSets[i];
		setWrites[3].descriptorCount = 1;
		setWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[3].pBufferInfo = bufferInfos.data() + 3;

		setWrites[4] = {};
		setWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[4].pNext = nullptr;
		setWrites[4].dstBinding = 4;
		setWrites[4].dstSet = m_compSets[i];
		setWrites[4].descriptorCount = 1;
		setWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setWrites[4].pBufferInfo = bufferInfos.data() + 4;

		setWrites[5] = {};
		setWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[5].pNext = nullptr;
		setWrites[5].dstBinding = 5;
		setWrites[5].dstSet = m_compSets[i];
		setWrites[5].descriptorCount = 1;
		setWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[5].pBufferInfo = bufferInfos.data() + 5;

		setWrites[6] = {};
		setWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[6].pNext = nullptr;
		setWrites[6].dstBinding = 6;
		setWrites[6].dstSet = m_compSets[i];
		setWrites[6].descriptorCount = 1;
		setWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[6].pBufferInfo = bufferInfos.data() + 6;


		vkUpdateDescriptorSets(m_boundedDevice->get_vk_device(), setWrites.size(), setWrites.data(), 0, nullptr);

	}

	//X Create VkPipelineLayout
	VkPipelineLayoutCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	inf.flags = 0;
	inf.setLayoutCount = 1;
	inf.pSetLayouts = &m_compSetLayout;
	inf.pushConstantRangeCount = 0;
	inf.pPushConstantRanges = 0;

	auto pipeRes = vkCreatePipelineLayout(m_boundedDevice->get_vk_device(), &inf, nullptr, &m_compPipeLayout);
	assert(pipeRes == VK_SUCCESS);

	VkComputePipelineCreateInfo compInfo = {};
	compInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	compInfo.pNext = 0;
	compInfo.flags = 0;
	compInfo.stage = *m_compStage->get_creation_info();
	compInfo.basePipelineHandle = 0;
	compInfo.basePipelineIndex = 0;
	compInfo.layout = m_compPipeLayout;

	auto cmpPipe = vkCreateComputePipelines(m_boundedDevice->get_vk_device(), nullptr, 1, &compInfo, nullptr, &m_compPipeline);
	assert(cmpPipe == VK_SUCCESS);
	//-----------------------------------------




	std::vector<IVulkanShaderStage*> stages;
	stages.push_back(m_vertStage);
	stages.push_back(m_fragStage);

	VkPipelineRasterizationStateCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	createInfo.depthClampEnable = VK_FALSE;
	//discards all primitives before the rasterization stage if enabled which we don't want
	createInfo.rasterizerDiscardEnable = VK_FALSE;
	createInfo.polygonMode = VK_POLYGON_MODE_FILL;
	createInfo.lineWidth = 1.0f;
	//no backface cull
	createInfo.cullMode = VK_CULL_MODE_NONE;
	createInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	//no depth bias
	createInfo.depthBiasEnable = VK_FALSE;
	createInfo.depthBiasConstantFactor = 0.0f;
	createInfo.depthBiasClamp = 0.0f;
	createInfo.depthBiasSlopeFactor = 0.0f;



	VkPipelineColorBlendAttachmentState colorAttachmentState = {};
	colorAttachmentState.blendEnable = true;
	colorAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT;
	colorAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorState = {};
	colorState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorState.logicOpEnable = VK_FALSE;
	colorState.logicOp = VK_LOGIC_OP_COPY;
	colorState.attachmentCount = 1;
	colorState.pAttachments = &colorAttachmentState;

	std::vector<IGVulkanGraphicPipelineState*> states;
	states.push_back(m_boundedDevice->create_vertex_input_state(nullptr, nullptr));
	states.push_back(m_boundedDevice->create_default_input_assembly_state());
	states.push_back(m_boundedDevice->create_default_none_multisample_state());
	states.push_back(m_boundedDevice->create_default_color_blend_state());
	states.push_back(m_boundedDevice->create_default_viewport_state(1920, 1080));
	states.push_back(m_boundedDevice->create_default_depth_stencil_state());
	states.push_back(m_boundedDevice->create_default_rasterization_state());

	m_pipeline = m_boundedDevice->create_and_init_graphic_pipeline_injector_for_vp(port, stages, states, m_layout);

	for (int i = 0; i < states.size(); i++)
	{
		delete states[i];
	}
	assert(m_pipeline != nullptr);
	

	return true;
}

void GMultiMeshRenderer::update_indirect_commands(uint32_t frameIndex)
{
	auto cmd = (VkDrawIndirectCommand*)m_indirectBuffers[frameIndex]->map_memory();
	assert(maxInstances_ == 1);
	for (uint32_t i = 0; i < maxInstances_; i++) {
		const uint32_t j = m_instanceDatas[i].mesh;
		cmd[i].vertexCount = m_meshDatas->meshes_[j].getLODIndicesCount(m_instanceDatas[i].lod);
		cmd[i].instanceCount = 0;
		cmd[i].firstVertex = 0;
		cmd[i].firstInstance = 0;
	}
	m_indirectBuffers[frameIndex]->unmap_memory();
}

void GMultiMeshRenderer::fill_command_buffer(GVulkanCommandBuffer* cmd,uint32_t frameIndex, IGVulkanViewport* vp)
{
	vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->get_pipeline());
	m_pipeline->bind_sets(cmd,frameIndex);

	
	vkCmdSetViewport(cmd->get_handle(), 0, 1, vp->get_viewport_area());
	vkCmdSetScissor(cmd->get_handle(), 0, 1, vp->get_scissor_area());

	vkCmdPushConstants(cmd->get_handle(), m_pipeline->get_pipeline_layout()->get_vk_pipeline_layout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(WireFrameSpec), &m_wireframeSpec);
	
	vkCmdDrawIndirect(cmd->get_handle(), m_indirectBuffers[frameIndex]->get_vk_buffer(), 0, maxInstances_, sizeof(VkDrawIndirectCommand));
}

void GMultiMeshRenderer::fill_command_buffer_dispatch(GVulkanCommandBuffer* cmd, uint32_t frameIndex)
{
	//X Update cull data
	auto cullData = m_cameraManager->get_cull_data();
	cullData->drawCount = m_indirectBuffers.size();
	
	memcpy(m_cullDataBufferMappedMem, cullData, sizeof(DrawCullData));

	//X Bind compute pipeline and create draw command
	vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_compPipeline);
	vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_compPipeLayout, 0, 1, &m_compSets[frameIndex], 0, 0);

	// Group counts will be increased for depth pyramit building
	vkCmdDispatch(cmd->get_handle(), 1,1,1);
}

std::vector<MaterialDescription>* GMultiMeshRenderer::get_global_materials()
{
	return &m_materialDatas;
}

WireFrameSpec* GMultiMeshRenderer::get_wireframe_spec()
{
	return &m_wireframeSpec;
}
