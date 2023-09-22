#include "volk.h"
#include <memory>
#include "internal/engine/rendering/renderer/gscene_renderer.h"
#include "engine/manager/igresource_manager.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include <vma/vk_mem_alloc.h>
#include "internal/engine/rendering/renderer/gscene_wireframe_renderer_layout.h"
#include "engine/manager/igshader_manager.h"
#include "engine/rendering/scene/scene.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"
#include <array>
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_pdevice.h"
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"
#include "engine/manager/igcamera_manager.h"
#include "engine/gengine.h"
#include "internal/engine/rendering/renderer/gscene_material_renderer_layout.h"
#include "internal/engine/rendering/renderer/gdeferred_renderer_layout.h"


#define MAX_BINDLESS_RESOURCES 1024
#define BINDLESS_TEXTURE_BINDING 10

GSceneRenderer::GSceneRenderer(IGVulkanLogicalDevice* device, IGCameraManager* cam, IGSceneManager* sceneMng, IGResourceManager* res, IGShaderManager* sm, uint32_t frameCount, Scene* scene, std::vector<MaterialDescription>& materials, MeshData* meshData)
{
	p_sceneManager = sceneMng;
	p_boundedDevice = device;
	p_cameraManager = cam;
	p_resourceManager = res;
	p_shaderManager = sm;
	m_framesInFlight = frameCount;
	m_scene = scene;
	m_materials = materials;
	m_meshData = meshData;
	m_wireframeSpec.thickness = 0.5f;
	m_wireframeSpec.step = 4.0f;

	m_deferredVp = new GVulkanNamedDeferredViewport(device, "deferred_viewport");
}


Scene* GSceneRenderer::get_global_scene()
{
	return m_scene;
}

MeshData* GSceneRenderer::get_global_mesh_data()
{
	return m_meshData;
}

bool GSceneRenderer::init(IGVulkanViewport* vp)
{
	m_vertexBufferInUsageSize = m_meshData->vertexData_.size() * sizeof(float);
	m_vertexBufferFullSize = m_vertexBufferInUsageSize;

	m_indexBufferInUsageSize = m_meshData->indexData_.size() * sizeof(uint32_t);
	m_indexBufferFullSize = m_indexBufferInUsageSize;

	//X Load the shaders

	m_normalVertexShader = p_resourceManager->create_shader_resource("normalVertexShader", "instancedGroup", "./indirect_mesh.glsl_vert").value();
	assert(m_normalVertexShader->load() == RESOURCE_INIT_CODE_OK);

	m_wireframeFragShader = p_resourceManager->create_shader_resource("wireframeFragShader", "instancedGroup", "./wireframe.glsl_frag").value();
	assert(m_wireframeFragShader->load() == RESOURCE_INIT_CODE_OK);

	m_materialFragShader = p_resourceManager->create_shader_resource("materialFragShader", "instancedGroup", "./indirect_mesh.glsl_frag").value();
	assert(m_materialFragShader->load() == RESOURCE_INIT_CODE_OK);
	
	m_cullCompShader = p_resourceManager->create_shader_resource("cullCompShader", "instancedGroup", "./mesh_command_creator.glsl_comp").value();
	assert(m_cullCompShader->load() == RESOURCE_INIT_CODE_OK);


	m_deferredVertexShader = p_resourceManager->create_shader_resource("deferredVertexShader", "instancedGroup", "./deferred.glsl_vert").value();
	assert(m_deferredVertexShader->load() == RESOURCE_INIT_CODE_OK);

	m_deferredPixelShader = p_resourceManager->create_shader_resource("deferredFragShader", "instancedGroup", "./deferred.glsl_frag").value();
	assert(m_deferredPixelShader->load() == RESOURCE_INIT_CODE_OK);

	m_composingVertexShader = p_resourceManager->create_shader_resource("composingVertexShader", "instancedGroup", "./composition.glsl_vert").value();
	assert(m_composingVertexShader->load() == RESOURCE_INIT_CODE_OK);

	m_composingPixelShader = p_resourceManager->create_shader_resource("composingFragShader", "instancedGroup", "./composition.glsl_frag").value();
	assert(m_composingPixelShader->load() == RESOURCE_INIT_CODE_OK);


	m_cullCompStage = p_shaderManager->create_shader_stage_from_shader_res(m_cullCompShader).value();
	m_deferredVertexStage = p_shaderManager->create_shader_stage_from_shader_res(m_deferredVertexShader).value();
	m_deferredFragStage = p_shaderManager->create_shader_stage_from_shader_res(m_deferredPixelShader).value();
	m_composingVertexStage = p_shaderManager->create_shader_stage_from_shader_res(m_composingVertexShader).value();
	m_composingFragStage = p_shaderManager->create_shader_stage_from_shader_res(m_composingPixelShader).value();
	//X Create buffers
	//X VBO
	if (m_vertexBufferFullSize == 0)
		m_vertexBufferFullSize = 1;
	m_mergedVertexBuffer.reset(p_boundedDevice->create_buffer(m_vertexBufferFullSize,VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_mergedVertexBuffer->copy_data_to_device_memory(m_meshData->vertexData_.data(), m_meshData->vertexData_.size() * sizeof(float));
	if (m_indexBufferFullSize == 0)
		m_indexBufferFullSize = 1;
	//X IBO
	m_mergedIndexBuffer.reset(p_boundedDevice->create_buffer(m_indexBufferFullSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_mergedIndexBuffer->copy_data_to_device_memory(m_meshData->indexData_.data(), m_meshData->indexData_.size() * sizeof(uint32_t));
	
	meshData.resize(m_meshData->meshes_.size());
	
	for (int i = 0; i < m_meshData->meshes_.size(); i++)
	{
		meshData[i].boundingBox = m_meshData->boxes_[i];
		meshData[i].extent = glm::vec4(1.f, 1.f, 1.f, 1.f);
		meshData[i].vertexCount = m_meshData->meshes_[i].vertexCount;
		meshData[i].vertexOffset = m_meshData->meshes_[i].vertexOffset;
		meshData[i].indexOffset = m_meshData->meshes_[i].indexOffset;
		meshData[i].lodCount = m_meshData->meshes_[i].lodCount;
		for (int j = 0; j < MeshConstants::MAX_LOD_COUNT;j++)
		{
			meshData[i].lodOffset[j] = m_meshData->meshes_[i].lodOffset[j];
		}
	}

	for (int i = 0; i < m_scene->hierarchy.size(); i++)
	{
		//X If there is a corresponded mesh add to drawList
		if (auto msh = m_scene->meshes_.find(i); msh != m_scene->meshes_.end())
		{
			auto currentData = drawDatas.size();
			drawDatas.push_back(DrawData{ .mesh = msh->second,.material = 0,.transformIndex = uint32_t(i) });

			//X Add transform data for it
			auto transformId = i;
			m_scene->globalTransform_[i] = glm::translate(glm::mat4(1.f), glm::vec3(-15.f, 0.f, 10.f)) * glm::scale(glm::mat4(1.f), glm::vec3(50, 50, 50));
			m_scene->localTransform_[i] = glm::translate(glm::mat4(1.f), glm::vec3(-15.f, 0.f, 10.f)) * glm::scale(glm::mat4(1.f), glm::vec3(50, 50, 50));
			drawDatas[currentData].transformIndex = i;
			//X If there is an assigned material give it
			if (auto mtrl = m_scene->materialForNode_.find(i); mtrl != m_scene->materialForNode_.end())
			{
				drawDatas[currentData].material = mtrl->second;
			}
		}
	}

	//X Upload Material Datas
	m_materialSSBO.reset(p_boundedDevice->create_buffer(m_materials.size() * sizeof(MaterialDescription), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_materialSSBO->copy_data_to_device_memory(m_materials.data(), m_materials.size() * sizeof(MaterialDescription));
	//X Upload transform datas
	m_transformDataSSBO.reset(p_boundedDevice->create_buffer(m_scene->globalTransform_.size() * sizeof(glm::mat4), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_transformDataMappedMem = m_transformDataSSBO->map_memory();
	memcpy(m_transformDataMappedMem,m_scene->globalTransform_.data(), m_scene->globalTransform_.size() * sizeof(glm::mat4));


	//x After calculating give these to gpu
	//X TODO : CHANGING SIZE DYNAMICALLY
	auto sizeOfMesh = meshData.size() * sizeof(GMeshData);
	if (sizeOfMesh == 0)
		sizeOfMesh = 1;
	m_meshDataSSBO.reset(p_boundedDevice->create_buffer(sizeOfMesh,VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_meshDataSSBO->copy_data_to_device_memory(meshData.data(), meshData.size() * sizeof(GMeshData));

	//X Draw Datas

		//X First one will stroe the count buffer
	auto sizeOfDrawData = drawDatas.size() * sizeof(DrawData);
	if (sizeOfDrawData == 0)
		sizeOfDrawData = 1;
	m_drawDataSSBO.reset((p_boundedDevice->create_buffer(sizeOfDrawData, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value()));
	m_drawDataSSBO->copy_data_to_device_memory(drawDatas.data(), drawDatas.size() * sizeof(DrawData));

	auto sizeOfDrawIDs = drawDatas.size() * sizeof(uint32_t);
	if (sizeOfDrawIDs == 0)
		sizeOfDrawIDs = 1;
	m_drawDataIDSSBO.reset((p_boundedDevice->create_buffer(sizeOfDrawIDs, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value()));
	void* dataid = m_drawDataIDSSBO->map_memory();
	memset(dataid, 0, drawDatas.size() * sizeof(uint32_t));
	m_drawDataIDSSBO->unmap_memory();


	m_maxIndirectDrawCommand = drawDatas.size();
	m_indirectCommandSSBOs.resize(m_framesInFlight);
	m_indirectCommandsBeginOffset = sizeof(DrawDataGlobalInfo);

	auto minStorageAlignment = p_boundedDevice->get_bounded_physical_device()->get_vk_properties()->limits.minStorageBufferOffsetAlignment;
	if ((m_indirectCommandsBeginOffset & (minStorageAlignment - 1)) != 0)
	{
		m_indirectCommandsBeginOffset = (m_indirectCommandsBeginOffset + minStorageAlignment) & ~(minStorageAlignment - 1);
	}

	m_drawGlobalData.drawCount = 2;
	m_drawGlobalData.meshCount = m_meshData->meshes_.size();

	m_indirectCommandStagingBuffer.reset(p_boundedDevice->create_buffer(
		(m_maxIndirectDrawCommand * sizeof(VkDrawIndexedIndirectCommand)) + m_indirectCommandsBeginOffset, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY).value());
	
	auto begin = m_indirectCommandStagingBuffer->map_memory();
	auto size = m_indirectCommandStagingBuffer->get_size();
	auto dataGlobal = (DrawDataGlobalInfo*)begin;
	dataGlobal->drawCount = 0;
	dataGlobal->meshCount = m_meshData->meshes_.size();
	auto indirectBegin = std::uint64_t(begin) + m_indirectCommandsBeginOffset;

	VkDrawIndexedIndirectCommand* iter = (VkDrawIndexedIndirectCommand*)indirectBegin;
	for (int i = 0; i < m_maxIndirectDrawCommand; i++)
	{
		auto meshIndex = drawDatas[i].mesh;
		int currentLOD = 0;
		iter += i;
		iter->firstInstance = i;
		//X S	elect first lod
		iter->firstIndex = 0;//meshData[meshIndex].indexOffset + (meshData[meshIndex].lodOffset[currentLOD] - meshData[meshIndex].lodOffset[0]);
		iter->indexCount = 0; //meshData[meshIndex].getLODIndicesCount(currentLOD);
		iter->instanceCount = 0;
		iter->vertexOffset = 0; //meshData[meshIndex].vertexOffset;
	}
	m_indirectCommandStagingBuffer->unmap_memory();


	for (int i = 0; i < m_framesInFlight; i++)
	{
		m_indirectCommandSSBOs[i].reset(p_boundedDevice->create_buffer(
			(m_maxIndirectDrawCommand * sizeof(VkDrawIndexedIndirectCommand))+ m_indirectCommandsBeginOffset, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY).value());
		
	}
	
	//X Cull Data
	m_cullDataUniform.reset(p_boundedDevice->create_buffer(sizeof(DrawCullData),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU).value());
	m_cullDataMappedMem = m_cullDataUniform->map_memory();
	//X Delete this when implemented 
	
	m_wireframeLayout = new GSceneWireframeRendererLayout(p_boundedDevice, m_framesInFlight, p_sceneManager, m_meshDataSSBO.get(),m_transformDataSSBO.get(), m_drawDataSSBO.get(), m_drawDataIDSSBO.get(), m_materialSSBO.get());

	m_normalVertexStage = p_shaderManager->create_shader_stage_from_shader_res(m_normalVertexShader).value();
	m_wireframeFragStage = p_shaderManager->create_shader_stage_from_shader_res(m_wireframeFragShader).value();
	m_materialFragStage = p_shaderManager->create_shader_stage_from_shader_res(m_materialFragShader).value();
	std::vector<IVulkanShaderStage*> stages;
	stages.push_back(m_normalVertexStage);
	stages.push_back(m_wireframeFragStage);

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
	
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(float)*7;
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].offset = 0;

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].offset = sizeof(float) * 3;

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].offset = sizeof(float) * 5;

	states.push_back(p_boundedDevice->create_vertex_input_state(&bindingDescriptions, &attributeDescriptions));
	states.push_back(p_boundedDevice->create_default_input_assembly_state());
	states.push_back(p_boundedDevice->create_default_none_multisample_state());
	states.push_back(p_boundedDevice->create_default_color_blend_state());
	states.push_back(p_boundedDevice->create_default_viewport_state(1920, 1080));
	states.push_back(p_boundedDevice->create_default_depth_stencil_state());
	states.push_back(p_boundedDevice->create_default_rasterization_state());

	m_wireframePipeline = p_boundedDevice->create_and_init_graphic_pipeline_injector_for_vp(vp, stages, states, m_wireframeLayout);
	


	assert(m_wireframePipeline != nullptr);


	//X Create Compute Pipeline

	std::unordered_map<VkDescriptorType, int> poolObjs;
	poolObjs.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
	poolObjs.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 5);

	m_compPool = p_boundedDevice->create_and_init_vector_pool(poolObjs, m_framesInFlight);
	std::array<VkDescriptorSetLayoutBinding, 8> bindings;
	//X Cam Buff
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	//X Counter BUFF
	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[1].pImmutableSamplers = nullptr;

	//X INDIRECT DRAW COMMAND BUFF
	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[2].pImmutableSamplers = nullptr;

	//X  MESH Data
	bindings[3].binding = 3;
	bindings[3].descriptorCount = 1;
	bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[3].pImmutableSamplers = nullptr;

	//X DRAW Data
	bindings[4].binding = 4;
	bindings[4].descriptorCount = 1;
	bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[4].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[4].pImmutableSamplers = nullptr;

	//X DRAW ID Data
	bindings[5].binding = 5;
	bindings[5].descriptorCount = 1;
	bindings[5].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[5].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[5].pImmutableSamplers = nullptr;

	//X Draw Cull Data
	bindings[6].binding = 6;
	bindings[6].descriptorCount = 1;
	bindings[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[6].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[6].pImmutableSamplers = nullptr;

	//X Transform Data
	bindings[7].binding = 7;
	bindings[7].descriptorCount = 1;
	bindings[7].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[7].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
	bindings[7].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pNext = nullptr;
	//we are going to have 1 binding
	setinfo.bindingCount = bindings.size();
	//no flags
	setinfo.flags = 0;
	//point to the camera buffer binding
	setinfo.pBindings = bindings.data();

	auto res = vkCreateDescriptorSetLayout(p_boundedDevice->get_vk_device(), &setinfo, nullptr, &m_compSetLayout);
	assert(res == VK_SUCCESS);

	std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight, m_compSetLayout);

	//allocate one descriptor set for each frame
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_compPool->get_vk_descriptor_pool();
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	m_compSets.resize(m_framesInFlight);

	assert(VK_SUCCESS == vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &allocInfo, m_compSets.data()));

	//X Update sets

	for (int i = 0; i < m_compSets.size(); i++)
	{
		auto gbuff = p_sceneManager->get_global_buffer_for_frame(i);
		std::array<VkDescriptorBufferInfo, 8> bufferInfos;
		//it will be the camera buffer
		bufferInfos[0].buffer = gbuff->get_vk_buffer();
		//at 0 offset
		bufferInfos[0].offset = 0;
		//of the size of a camera data struct
		bufferInfos[0].range = gbuff->get_size();
		//X Global Data
		bufferInfos[1].buffer = m_indirectCommandSSBOs[i]->get_vk_buffer();
		bufferInfos[1].offset = 0;
		bufferInfos[1].range = m_indirectCommandsBeginOffset;

		//X Indirect Command Data
		bufferInfos[2].buffer = m_indirectCommandSSBOs[i]->get_vk_buffer();
		bufferInfos[2].offset = m_indirectCommandsBeginOffset;
		bufferInfos[2].range = m_indirectCommandSSBOs[i]->get_size() - m_indirectCommandsBeginOffset;

		//X Mesh Data
		bufferInfos[3].buffer = m_meshDataSSBO->get_vk_buffer();
		bufferInfos[3].offset = 0;
		bufferInfos[3].range = m_meshDataSSBO->get_size();

		//X Draw Data
		bufferInfos[4].buffer = m_drawDataSSBO->get_vk_buffer();
		bufferInfos[4].offset = 0;
		bufferInfos[4].range = m_drawDataSSBO->get_size();

		//X DRAW ID Data
		bufferInfos[5].buffer = m_drawDataIDSSBO->get_vk_buffer();
		bufferInfos[5].offset = 0;
		bufferInfos[5].range = m_drawDataIDSSBO->get_size();

		//X Cull Uniform Data
		bufferInfos[6].buffer = m_cullDataUniform->get_vk_buffer();
		bufferInfos[6].offset = 0;
		bufferInfos[6].range = m_cullDataUniform->get_size();

		//X Transform Data
		bufferInfos[7].buffer = m_transformDataSSBO->get_vk_buffer();
		bufferInfos[7].offset = 0;
		bufferInfos[7].range = m_transformDataSSBO->get_size();

		std::array< VkWriteDescriptorSet, 8> setWrites;
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
		setWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
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
		setWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setWrites[6].pBufferInfo = bufferInfos.data() + 6;

		setWrites[7] = {};
		setWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[7].pNext = nullptr;
		setWrites[7].dstBinding = 7;
		setWrites[7].dstSet = m_compSets[i];
		setWrites[7].descriptorCount = 1;
		setWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		setWrites[7].pBufferInfo = bufferInfos.data() + 7;

		vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), setWrites.size(), setWrites.data(), 0, nullptr);

	}

	//X Create VkPipelineLayout
	VkPipelineLayoutCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	inf.flags = 0;
	inf.setLayoutCount = 1;
	inf.pSetLayouts = &m_compSetLayout;
	inf.pushConstantRangeCount = 0;
	inf.pPushConstantRanges = 0;

	auto pipeRes = vkCreatePipelineLayout(p_boundedDevice->get_vk_device(), &inf, nullptr, &m_compPipeLayout);
	assert(pipeRes == VK_SUCCESS);

	VkComputePipelineCreateInfo compInfo = {};
	compInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	compInfo.pNext = 0;
	compInfo.flags = 0;
	compInfo.stage = *m_cullCompStage->get_creation_info();
	compInfo.basePipelineHandle = 0;
	compInfo.basePipelineIndex = 0;
	compInfo.layout = m_compPipeLayout;

	auto cmpPipe = vkCreateComputePipelines(p_boundedDevice->get_vk_device(), nullptr, 1, &compInfo, nullptr, &m_compPipeline);
	assert(cmpPipe == VK_SUCCESS);

	assert(init_bindless());

	stages[1] = m_materialFragStage;
	m_materialLayout = new GSceneMaterialRendererLayout(p_boundedDevice,m_bindlessSetLayout,m_wireframeLayout->get_set_layout(), m_wireframeLayout->get_sets());
	m_materialPipeline = p_boundedDevice->create_and_init_graphic_pipeline_injector_for_vp(vp, stages, states, m_materialLayout);


	m_deferredLayout = new GSceneDeferredRendererLayout(p_boundedDevice, m_wireframeLayout->get_set_layout(), m_bindlessSetLayout,m_wireframeLayout->get_sets());
	stages[0] = m_deferredVertexStage;
	stages[1] = m_deferredFragStage;

	std::array< VkPipelineColorBlendAttachmentState, 3> attachmentStates;
	attachmentStates[0] = {};
	attachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	attachmentStates[0].blendEnable = VK_FALSE;

	attachmentStates[1] = {};
	attachmentStates[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	attachmentStates[1].blendEnable = VK_FALSE;

	attachmentStates[2] = {};
	attachmentStates[2].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	attachmentStates[2].blendEnable = VK_FALSE;
	VkPipelineColorBlendStateCreateInfo	bcreateInfo = {};
	bcreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	bcreateInfo.logicOpEnable = VK_FALSE;
	bcreateInfo.logicOp = VK_LOGIC_OP_COPY;
	bcreateInfo.attachmentCount = attachmentStates.size();
	bcreateInfo.pAttachments = attachmentStates.data();
	
	
	delete states[3];

	states[3] = p_boundedDevice->create_custom_color_blend_state(attachmentStates.data(), &bcreateInfo);

	m_deferredPipeline = p_boundedDevice->create_and_init_graphic_pipeline_injector_for_renderpass(m_deferredVp->get_dedicated_renderpass(), stages, states, m_framesInFlight, m_deferredLayout);

	m_compositionLayout = new GSceneCompositionRendererLayout(p_boundedDevice, m_framesInFlight, m_deferredVp->get_named_attachment("position_attachment"),
		m_deferredVp->get_named_attachment("normal_attachment"), m_deferredVp->get_named_attachment("albedo_attachment"), m_deferredVp->get_sampler_for_named_attachment("position_attachment"));

	delete states[0];
	states[0] = p_boundedDevice->create_vertex_input_state(nullptr, nullptr);
	
	delete states[3];
	states[3] = p_boundedDevice->create_default_color_blend_state();

	stages[0] = m_composingFragStage;
	stages[1] = m_composingVertexStage;

	m_composingPipeline = p_boundedDevice->create_and_init_graphic_pipeline_injector_for_renderpass(m_deferredVp->get_named_renderpass(SECOND_RENDER_PASS.data()), stages, states, m_framesInFlight, m_compositionLayout);

	for (int i = 0; i < states.size(); i++)
	{
		delete states[i];
	}

	return true;
}

void GSceneRenderer::destroy()
{
	for (int i = 0; i < m_indirectCommandSSBOs.size(); i++)
	{
		m_indirectCommandSSBOs[i]->unmap_memory();
		m_indirectCommandSSBOs[i]->unload();
	}
}

bool GSceneRenderer::init_deferred(uint32_t width, uint32_t height)
{
	return m_deferredVp->init(width,height);
}

bool GSceneRenderer::resize_deferred(uint32_t width, uint32_t height)
{
	auto t = m_deferredVp->resize(width,height);
	if (!t)
		return t;

	m_compositionLayout->write_sets(m_deferredVp->get_named_attachment("position_attachment"),
		m_deferredVp->get_named_attachment("normal_attachment"), m_deferredVp->get_named_attachment("albedo_attachment"));
}

IGVulkanNamedViewport* GSceneRenderer::get_deferred_vp()
{
	return m_deferredVp;
}

void GSceneRenderer::update_scene_nodes()
{
	bool recalculated = m_scene->recalculate_transforms();
	if (recalculated)
	{
		while (!m_scene->changedNodesAtThisFrame_.empty())
		{
			int nodeIndex = m_scene->changedNodesAtThisFrame_.front();
			m_scene->changedNodesAtThisFrame_.pop();
			glm::mat4* offset = ((glm::mat4*)m_transformDataMappedMem) + nodeIndex;
			memcpy(offset,&m_scene->globalTransform_[nodeIndex],sizeof(glm::mat4));
		}
	}
}

void GSceneRenderer::fill_command_buffer(GVulkanCommandBuffer* cmd, uint32_t frameIndex, IGVulkanViewport* vp)
{

	if (!isWireframeInUse)
	{
		vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_wireframePipeline->get_pipeline());
		m_wireframePipeline->bind_sets(cmd, frameIndex);
		vkCmdPushConstants(cmd->get_handle(), m_wireframePipeline->get_pipeline_layout()->get_vk_pipeline_layout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(WireFrameSpec), &m_wireframeSpec);

	}
	else
	{
		vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_materialPipeline->get_pipeline());
		std::array<VkDescriptorSet, 2> sets;
		sets[0] = m_deferredLayout->get_set_by_index(frameIndex);
		sets[1] = m_bindlessSet;
		vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_materialLayout->get_pipeline_layout()->get_vk_pipeline_layout(), 0, sets.size(), sets.data(), 0, nullptr);
	}


	vkCmdSetViewport(cmd->get_handle(), 0, 1, vp->get_viewport_area());
	vkCmdSetScissor(cmd->get_handle(), 0, 1, vp->get_scissor_area());

	
	VkBuffer vertBuff = m_mergedVertexBuffer->get_vk_buffer();

	VkDeviceSize deviceOffset = 0;

	vkCmdBindVertexBuffers(cmd->get_handle(), 0, 1, &vertBuff,&deviceOffset);

	vkCmdBindIndexBuffer(cmd->get_handle(), m_mergedIndexBuffer->get_vk_buffer(), 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexedIndirectCount(cmd->get_handle(), m_indirectCommandSSBOs[frameIndex]->get_vk_buffer(),m_indirectCommandsBeginOffset, 
		m_indirectCommandSSBOs[frameIndex]->get_vk_buffer(),0, m_maxIndirectDrawCommand,sizeof(VkDrawIndexedIndirectCommand));
}

void GSceneRenderer::fill_command_deferred(GVulkanCommandBuffer* cmd, uint32_t frameIndex)
{
	m_deferredVp->begin_draw_cmd(cmd);
	vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_deferredPipeline->get_pipeline());
	std::array<VkDescriptorSet, 2> sets;
	sets[0] = m_deferredLayout->get_set_by_index(frameIndex);
	sets[1] = m_bindlessSet;
	vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_deferredLayout->get_pipeline_layout(), 0, sets.size(), sets.data(), 0, nullptr);

vkCmdSetViewport(cmd->get_handle(), 0, 1, m_deferredVp->get_viewport_area());
vkCmdSetScissor(cmd->get_handle(), 0, 1, m_deferredVp->get_scissor_area());


VkBuffer vertBuff = m_mergedVertexBuffer->get_vk_buffer();

VkDeviceSize deviceOffset = 0;

vkCmdBindVertexBuffers(cmd->get_handle(), 0, 1, &vertBuff, &deviceOffset);

vkCmdBindIndexBuffer(cmd->get_handle(), m_mergedIndexBuffer->get_vk_buffer(), 0, VK_INDEX_TYPE_UINT32);

vkCmdDrawIndexedIndirectCount(cmd->get_handle(), m_indirectCommandSSBOs[frameIndex]->get_vk_buffer(), m_indirectCommandsBeginOffset,
	m_indirectCommandSSBOs[frameIndex]->get_vk_buffer(), 0, m_maxIndirectDrawCommand, sizeof(VkDrawIndexedIndirectCommand));


m_deferredVp->end_draw_cmd(cmd);

//X Composition
	m_deferredVp->begin_draw_cmd_to_named_pass(cmd, SECOND_RENDER_PASS.data());
	vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_composingPipeline->get_pipeline());
	m_composingPipeline->bind_sets(cmd, frameIndex);

	vkCmdSetViewport(cmd->get_handle(), 0, 1, m_deferredVp->get_viewport_area());
	vkCmdSetScissor(cmd->get_handle(), 0, 1, m_deferredVp->get_scissor_area());
	
	vkCmdDraw(cmd->get_handle(), 3, 1, 0, 0);


	m_deferredVp->end_draw_cmd(cmd);
}

void GSceneRenderer::fill_command_buffer_dispatch(GVulkanCommandBuffer* cmd, uint32_t frameIndex)
{
	//X Clear Count Buffer
	VkBufferCopy cpy = {};
	cpy.srcOffset = 0;
	cpy.dstOffset = 0;
	VkBufferMemoryBarrier barr = {};
	barr.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barr.pNext = nullptr;
	barr.srcAccessMask = 0;
	barr.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barr.buffer = m_indirectCommandSSBOs[frameIndex]->get_vk_buffer();
	barr.size = m_indirectCommandStagingBuffer->get_size();

	cpy.size = m_indirectCommandStagingBuffer->get_size();
	vkCmdPipelineBarrier(cmd->get_handle(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, 0,0,0,1,&barr,0,0);
	vkCmdCopyBuffer(cmd->get_handle(), m_indirectCommandStagingBuffer->get_vk_buffer(), m_indirectCommandSSBOs[frameIndex]->get_vk_buffer(), 1, &cpy);
	barr.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barr.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(cmd->get_handle(), VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, 0, 1, &barr, 0, 0);
	//X Update Cull Data
	auto cullData = p_cameraManager->get_cull_data();
	cullData->drawCount = m_maxIndirectDrawCommand;
	memcpy(m_cullDataMappedMem, cullData, sizeof(DrawCullData));

	//X Bind compute pipeline and create draw command
	vkCmdBindPipeline(cmd->get_handle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_compPipeline);
	vkCmdBindDescriptorSets(cmd->get_handle(), VK_PIPELINE_BIND_POINT_COMPUTE, m_compPipeLayout, 0, 1, &m_compSets[frameIndex], 0, 0);

	// Group counts will be increased for depth pyramit building
	vkCmdDispatch(cmd->get_handle(), 10, 1, 1);

	barr.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
	barr.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;

	vkCmdPipelineBarrier(cmd->get_handle(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
		VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT, 0, 0, 0, 1, &barr, 0, 0);
}

int GSceneRenderer::add_mesh(MeshData* mesh)
{
	//xInsert mesh material draw data
	//X Resize vertex and index buffers in cpu
	auto vertOffset = m_meshData->vertexData_.size();
	m_meshData->vertexData_.resize(m_meshData->vertexData_.size() + mesh->vertexData_.size());
	memcpy(&m_meshData->vertexData_[vertOffset], mesh->vertexData_.data(), mesh->vertexData_.size() * sizeof(float));
	auto indexOffset = m_meshData->indexData_.size();
	m_meshData->indexData_.resize(m_meshData->indexData_.size() + mesh->indexData_.size());
	memcpy(&m_meshData->indexData_[indexOffset],mesh->indexData_.data(),mesh->indexData_.size()*sizeof(uint32_t));

	//X Add bounding boxes to the vector
	int boxBegin = m_meshData->boxes_.size();
	m_meshData->boxes_.resize(boxBegin + mesh->boxes_.size());
	memcpy(&m_meshData->boxes_[boxBegin],mesh->boxes_.data(), mesh->boxes_.size() * sizeof(BoundingBox));

	//X Set the offset right
	int meshBegin = m_meshData->meshes_.size();
	m_meshData->meshes_.resize(meshBegin + mesh->meshes_.size());
	memcpy(&m_meshData->meshes_[meshBegin], mesh->meshes_.data(),mesh->meshes_.size() * sizeof(GMesh));

	for (int i = meshBegin; i < m_meshData->meshes_.size(); i++)
	{
		m_meshData->meshes_[i].vertexOffset += (vertOffset/7);
		m_meshData->meshes_[i].indexOffset += indexOffset;
	}

	//X Copy data to gpu
	GEngine::get_instance()->add_recreation([&]() {
		//X TODO : DONT RECREATE ALWASY PREALLOCATE ETC
		m_mergedVertexBuffer->unload();
		m_mergedIndexBuffer->unload();
		m_meshDataSSBO->unload();
		m_vertexBufferFullSize = m_meshData->vertexData_.size() * sizeof(float);
		m_vertexBufferInUsageSize = m_vertexBufferFullSize;
		m_indexBufferFullSize = m_meshData->indexData_.size() * sizeof(uint32_t);
		m_indexBufferInUsageSize = m_indexBufferFullSize;

		//X VBO
		m_mergedVertexBuffer.reset(p_boundedDevice->create_buffer(m_vertexBufferFullSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value());
		m_mergedVertexBuffer->copy_data_to_device_memory(m_meshData->vertexData_.data(), m_meshData->vertexData_.size() * sizeof(float));

		//X IBO
		m_mergedIndexBuffer.reset(p_boundedDevice->create_buffer(m_indexBufferFullSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value());
		m_mergedIndexBuffer->copy_data_to_device_memory(m_meshData->indexData_.data(), m_meshData->indexData_.size() * sizeof(uint32_t));

		//X Update GMeshData
		meshData.clear();
		meshData.resize(m_meshData->meshes_.size());

		for (int i = 0; i < m_meshData->meshes_.size(); i++)
		{
			meshData[i].boundingBox = m_meshData->boxes_[i];
			meshData[i].extent = glm::vec4(1.f, 1.f, 1.f, 1.f);
			meshData[i].vertexCount = m_meshData->meshes_[i].vertexCount;
			meshData[i].vertexOffset = m_meshData->meshes_[i].vertexOffset;
			meshData[i].indexOffset = m_meshData->meshes_[i].indexOffset;
			meshData[i].lodCount = m_meshData->meshes_[i].lodCount;
			for (int j = 0; j < MeshConstants::MAX_LOD_COUNT; j++)
			{
				meshData[i].lodOffset[j] = m_meshData->meshes_[i].lodOffset[j];
			}
		}

		m_meshDataSSBO.reset(p_boundedDevice->create_buffer(meshData.size() * sizeof(GMeshData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value());
		m_meshDataSSBO->copy_data_to_device_memory(meshData.data(), meshData.size() * sizeof(GMeshData));

		//X Write to compute sets
		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = m_meshDataSSBO->get_vk_buffer();
		bufferInfo.offset = 0;
		bufferInfo.range = m_meshDataSSBO->get_size();
		for (int i = 0; i < m_framesInFlight; i++)
		{
			VkWriteDescriptorSet setWrite;
			setWrite = {};
			setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			setWrite.pNext = nullptr;
			setWrite.dstBinding = 3;
			setWrite.dstSet = m_compSets[i];
			setWrite.descriptorCount = 1;
			setWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			setWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), 1, &setWrite, 0, nullptr);
		}

		//X Update for layout
		m_wireframeLayout->write_set_layout(3,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&bufferInfo);
	});
	return meshBegin;
}

std::vector<MaterialDescription>* GSceneRenderer::get_global_materials()
{
	return &m_materials;
}

WireFrameSpec* GSceneRenderer::get_wireframe_spec()
{
	return &m_wireframeSpec;
}

uint32_t GSceneRenderer::add_node_with_mesh(uint32_t meshIndex)
{
	uint32_t newNodeID = Scene::add_node(*m_scene, 0, m_scene->hierarchy[0].level + 1);
	GEngine::get_instance()->add_recreation([&,nodeID = newNodeID,selectedMeshIndex = meshIndex]() {
		m_scene->meshes_.emplace(nodeID, selectedMeshIndex);
		uint32_t transformID = nodeID;

		//x First recreate the draw commands
		m_maxIndirectDrawCommand++;
		auto currentData = drawDatas.size();
		drawDatas.push_back(DrawData{ .mesh = selectedMeshIndex,.material = 0,.transformIndex = nodeID });
		m_scene->drawIdForNode[nodeID] = currentData;
		//X Reset
		//X DRAWDATASSBO
		//X DRAWDATAIDSSBO
		//X TRANSFORMSSBO
		//X INDIRECTCOMMANDBUFFER
		//X INDIRECTCOMMANDBUFFERCLEAR

		m_drawDataIDSSBO->unload();
		m_drawDataSSBO->unload();
		m_transformDataSSBO->unmap_memory();
		m_transformDataSSBO->unload();

		m_drawDataSSBO.reset((p_boundedDevice->create_buffer((drawDatas.size() * sizeof(DrawData)), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value()));
		m_drawDataSSBO->copy_data_to_device_memory(drawDatas.data(), drawDatas.size() * sizeof(DrawData));

		m_drawDataIDSSBO.reset((p_boundedDevice->create_buffer((drawDatas.size() * sizeof(uint32_t)), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value()));
		void* dataid = m_drawDataIDSSBO->map_memory();
		memset(dataid, 0, drawDatas.size() * sizeof(uint32_t));
		m_drawDataIDSSBO->unmap_memory();

		
		m_transformDataSSBO.reset(p_boundedDevice->create_buffer(m_scene->globalTransform_.size() * sizeof(glm::mat4), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value());
		m_transformDataMappedMem = m_transformDataSSBO->map_memory();
		memcpy(m_transformDataMappedMem, m_scene->globalTransform_.data(), m_scene->globalTransform_.size() * sizeof(glm::mat4));


		for (int i = 0; i < m_framesInFlight; i++)
		{
			m_indirectCommandSSBOs[i]->unload();
			m_indirectCommandSSBOs[i].reset(p_boundedDevice->create_buffer(
				(m_maxIndirectDrawCommand * sizeof(VkDrawIndexedIndirectCommand)) + m_indirectCommandsBeginOffset, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY).value());

		}
		m_indirectCommandStagingBuffer->unload();
		m_indirectCommandStagingBuffer.reset(p_boundedDevice->create_buffer(
			(m_maxIndirectDrawCommand * sizeof(VkDrawIndexedIndirectCommand)) + m_indirectCommandsBeginOffset, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY).value());

		void* mem = m_indirectCommandStagingBuffer->map_memory();
		memset(mem, 0, (m_maxIndirectDrawCommand * sizeof(VkDrawIndexedIndirectCommand)) + m_indirectCommandsBeginOffset);
		m_indirectCommandStagingBuffer->unmap_memory();

		//X Update Cull Sets
		std::array<VkDescriptorBufferInfo, 5> bufferInfos;
		//X Draw Data
		bufferInfos[2].buffer = m_drawDataSSBO->get_vk_buffer();
		bufferInfos[2].offset = 0;
		bufferInfos[2].range = m_drawDataSSBO->get_size();

		//X DRAW ID Data
		bufferInfos[3].buffer = m_drawDataIDSSBO->get_vk_buffer();
		bufferInfos[3].offset = 0;
		bufferInfos[3].range = m_drawDataIDSSBO->get_size();

		//X Transform Data
		bufferInfos[4].buffer = m_transformDataSSBO->get_vk_buffer();
		bufferInfos[4].offset = 0;
		bufferInfos[4].range = m_transformDataSSBO->get_size();

		for(int i = 0;i<m_framesInFlight;i++)
		{
			//X Global Data
			bufferInfos[0].buffer = m_indirectCommandSSBOs[i]->get_vk_buffer();
			bufferInfos[0].offset = 0;
			bufferInfos[0].range = m_indirectCommandsBeginOffset;

			//X Indirect Command Data
			bufferInfos[1].buffer = m_indirectCommandSSBOs[i]->get_vk_buffer();
			bufferInfos[1].offset = m_indirectCommandsBeginOffset;
			bufferInfos[1].range = m_indirectCommandSSBOs[i]->get_size() - m_indirectCommandsBeginOffset;


			std::array< VkWriteDescriptorSet,5> setWrites;
			setWrites[0] = {};
			setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			setWrites[0].pNext = nullptr;
			setWrites[0].dstBinding = 1;
			setWrites[0].dstSet = m_compSets[i];
			setWrites[0].descriptorCount = 1;
			setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			setWrites[0].pBufferInfo = bufferInfos.data();

			setWrites[1] = {};
			setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			setWrites[1].pNext = nullptr;
			setWrites[1].dstBinding = 2;
			setWrites[1].dstSet = m_compSets[i];
			setWrites[1].descriptorCount = 1;
			setWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			setWrites[1].pBufferInfo = bufferInfos.data()+1;

			setWrites[2] = {};
			setWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			setWrites[2].pNext = nullptr;
			setWrites[2].dstBinding = 4;
			setWrites[2].dstSet = m_compSets[i];
			setWrites[2].descriptorCount = 1;
			setWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			setWrites[2].pBufferInfo = bufferInfos.data() + 2;

			setWrites[3] = {};
			setWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			setWrites[3].pNext = nullptr;
			setWrites[3].dstBinding = 5;
			setWrites[3].dstSet = m_compSets[i];
			setWrites[3].descriptorCount = 1;
			setWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			setWrites[3].pBufferInfo = bufferInfos.data() + 3;

			setWrites[4] = {};
			setWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			setWrites[4].pNext = nullptr;
			setWrites[4].dstBinding = 7;
			setWrites[4].dstSet = m_compSets[i];
			setWrites[4].descriptorCount = 1;
			setWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			setWrites[4].pBufferInfo = bufferInfos.data() + 4;

			vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), setWrites.size(), setWrites.data(), 0, nullptr);
		}

		m_wireframeLayout->write_set_layout(2,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,&bufferInfos[2]);
		m_wireframeLayout->write_set_layout(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &bufferInfos[3]);
		m_wireframeLayout->write_set_layout(5, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &bufferInfos[4]);

	});

	return newNodeID;
}

bool GSceneRenderer::create_deferred_resources()
{
	return true;
}

uint32_t GSceneRenderer::add_texture(IGTextureResource* res)
{
	uint32_t textureID = m_textureSize;
	m_textureSize++;
	GEngine::get_instance()->add_recreation([&,textureRes = res,textId = textureID]() {
		VkWriteDescriptorSet set = {};
		std::array<VkDescriptorImageInfo, 1> imageInfos;
		auto sampler = m_deferredVp->get_sampler_for_named_attachment("");
		//it will be the camera buffer
		imageInfos[0].imageView = textureRes->get_vulkan_image()->get_vk_image_view();
		imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos[0].sampler = sampler;

		std::array< VkWriteDescriptorSet, 1> setWrites;
		setWrites[0] = {};
		setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[0].pNext = nullptr;
		setWrites[0].dstBinding = BINDLESS_TEXTURE_BINDING;
		setWrites[0].dstSet = m_bindlessSet;
		setWrites[0].descriptorCount = 1;
		setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setWrites[0].pImageInfo = &imageInfos[0];
		setWrites[0].dstArrayElement = textId;

		vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), setWrites.size(), setWrites.data(), 0, nullptr);

	});

	return textureID;
}

bool GSceneRenderer::init_bindless()
{
	uint32_t poolSize = 2;;
	VkDescriptorPoolSize pool_sizes_bindless[] =
	{
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		  MAX_BINDLESS_RESOURCES },
		  { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		  MAX_BINDLESS_RESOURCES },
	};
	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
	pool_info.maxSets = MAX_BINDLESS_RESOURCES * m_framesInFlight;
	pool_info.poolSizeCount = poolSize;
	pool_info.pPoolSizes = pool_sizes_bindless;
	assert( VK_SUCCESS == vkCreateDescriptorPool(p_boundedDevice->get_vk_device(), &pool_info,
		nullptr,
		&m_bindlessPool));


	std::array<VkDescriptorSetLayoutBinding, 2> bindings;
	//X TEXTURE
	bindings[0].binding = BINDLESS_TEXTURE_BINDING;
	bindings[0].descriptorCount = MAX_BINDLESS_RESOURCES;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	//X Image Buff
	bindings[1].binding = BINDLESS_TEXTURE_BINDING+1;
	bindings[1].descriptorCount = MAX_BINDLESS_RESOURCES;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[1].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layout_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layout_info.bindingCount = bindings.size();
	layout_info.pBindings = bindings.data();
	layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
	//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
	VkDescriptorBindingFlags bindless_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
	VkDescriptorBindingFlags binding_flags[2];
	binding_flags[0] = bindless_flags;
	binding_flags[1] = bindless_flags;

	VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT, nullptr };
	extended_info.bindingCount = bindings.size();
	extended_info.pBindingFlags = binding_flags;

	layout_info.pNext = &extended_info;

	assert(VK_SUCCESS == vkCreateDescriptorSetLayout(p_boundedDevice->get_vk_device(), &layout_info, nullptr, &m_bindlessSetLayout));
	VkDescriptorSetAllocateInfo alloc_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	alloc_info.descriptorPool = m_bindlessPool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &m_bindlessSetLayout;

	VkDescriptorSetVariableDescriptorCountAllocateInfoEXT count_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT };
	uint32_t max_binding = MAX_BINDLESS_RESOURCES - 1;
	count_info.descriptorSetCount = 1;
	// This number is the max allocatable count
	count_info.pDescriptorCounts = &max_binding;
	alloc_info.pNext = &count_info;
	
	assert(VK_SUCCESS == vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &alloc_info, &m_bindlessSet));

	return true;
}

uint32_t GSceneRenderer::add_material(const MaterialDescription& desc)
{
	uint32_t materialIndex = m_materials.size();
	m_materials.push_back(desc);
	GEngine::get_instance()->add_recreation([&]() {
		m_materialSSBO->unload();
		m_materialSSBO.reset(p_boundedDevice->create_buffer(m_materials.size() * sizeof(MaterialDescription), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value());
		m_materialSSBO->copy_data_to_device_memory(m_materials.data(), m_materials.size() * sizeof(MaterialDescription));
		VkDescriptorBufferInfo inf = {};
		inf.offset = 0;
		inf.range = m_materialSSBO->get_size();
		inf.buffer = m_materialSSBO->get_vk_buffer();
		m_wireframeLayout->write_set_layout(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &inf);
	});

	return materialIndex;
}

void GSceneRenderer::set_material_for_node(uint32_t node,uint32_t material)
{
	GEngine::get_instance()->add_recreation([&,nodeID = node,materialIndex = material]() {
	if (auto drawIDIter = m_scene->drawIdForNode.find(nodeID); drawIDIter != m_scene->drawIdForNode.end())
	{
		uint32_t drawID = drawIDIter->second;
		auto drawDataMap = (DrawData*)m_drawDataSSBO->map_memory();
		drawDataMap[drawID].material = materialIndex;
		m_drawDataSSBO->unmap_memory();
	}
	});
	
}

void GSceneRenderer::resize_vertex_buffer(uint32_t newSize)
{
	m_mergedVertexBuffer->unload();
	m_vertexBufferFullSize = newSize;
	m_vertexBufferInUsageSize = 0;
	m_mergedVertexBuffer.reset(p_boundedDevice->create_buffer(m_vertexBufferFullSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value());
}

void GSceneRenderer::resize_index_buffer(uint32_t newSize)
{
	m_mergedIndexBuffer->unload();
	m_indexBufferFullSize = newSize;
	m_indexBufferInUsageSize = 0;
	m_mergedIndexBuffer.reset(p_boundedDevice->create_buffer(m_indexBufferFullSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value());
}
