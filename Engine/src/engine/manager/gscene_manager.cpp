#include "internal/engine/manager/gscene_manager.h"
#include "engine/gengine.h"
#include "engine/manager/igcamera_manager.h"
#include "engine/imanager_table.h"
#include "public/core/templates/shared_ptr.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"
#include <array>
#include "internal/engine/rendering/vulkan/named/viewports/gvulkan_named_base_deferred_viewport.h"
#include "vma/vk_mem_alloc.h"
#include <glm/ext.hpp>
#include <glm/gtx/matrix_decompose.hpp>

IGVulkanUniformBuffer* GSceneManager::get_global_buffer_for_frame(uint32_t frame) const noexcept
{
	return m_globalBuffers[frame];
}

void GSceneManager::reconstruct_global_buffer_for_frame(uint32_t frame)
{
	//X Copy datas to cpu
	memcpy(&m_globalData.viewProj[0], m_cameraManager->get_camera_view_proj_matrix(), 16 * sizeof(float));
	memcpy(&m_globalData.view[0], m_cameraManager->get_camera_view_matrix(), 16 * sizeof(float));
	memcpy(&m_globalData.pos[0], m_cameraManager->get_camera_position(), 3 * sizeof(float));
	m_globalData.resolution[0] = m_deferredTargetedViewport->get_viewport_area()->width;
	m_globalData.resolution[1] = m_deferredTargetedViewport->get_viewport_area()->height;


	//X Copy data to gpu
	memcpy(m_globalBufferMappedMem[frame], &m_globalData,sizeof(GlobalUniformBuffer));
	
	//X Update Cull Data
	auto cullData = *m_cameraManager->get_cull_data();
	m_deferredRenderer->update_cull_data(cullData);

	//X Update Global Transformations
	bool recalculated = m_currentScene->recalculate_transforms();
	if (recalculated)
	{
		while (!m_currentScene->changedNodesAtThisFrame_.empty())
		{
			int nodeIndex = m_currentScene->changedNodesAtThisFrame_.front();
			m_currentScene->changedNodesAtThisFrame_.pop();
			auto gpuIndexIter = m_cpu_to_gpu_map.find(nodeIndex);
			//X Data needed to update also in the gpu
			if (gpuIndexIter != m_cpu_to_gpu_map.end())
			{
				auto gpuIndex = gpuIndexIter->second;
				set_transform_by_index(&m_currentScene->globalTransform_[nodeIndex], gpuIndex);
			}
			if (auto light = m_nodeToLight.find(nodeIndex); light != m_nodeToLight.end())
			{
				auto lightIndex = light->second;
				auto lightData = m_globalPointLights.cpuVector[lightIndex];
				glm::vec3 nodePos = glm::vec3(m_currentScene->globalTransform_[nodeIndex][3][0], m_currentScene->globalTransform_[nodeIndex][3][1], m_currentScene->globalTransform_[nodeIndex][3][2]);
				lightData.position = nodePos;
				m_globalPointLights.set_by_index(&lightData, lightIndex);
			}
		}
	}
}

bool GSceneManager::init(uint32_t framesInFlight)
{
	auto pipelineManager = ((GSharedPtr<IGPipelineObjectManager>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_PIPELINE_OBJECT))->get();

	m_framesInFlight = framesInFlight;
	auto table = GEngine::get_instance()->get_manager_table();
	m_cameraManager = ((GSharedPtr<IGCameraManager>*)table->get_engine_manager_managed(ENGINE_MANAGER_CAMERA))->get();
	m_logicalDevice = ((GSharedPtr<IGVulkanDevice>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_GRAPHIC_DEVICE))->get()->as_logical_device().get();
	auto resManager = ((GSharedPtr<IGResourceManager>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE))->get();
	auto shaderManager = ((GSharedPtr<IGShaderManager>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SHADER))->get();

	for (int i = 0; i < m_framesInFlight; i++)
	{
		auto buffRes = m_logicalDevice->create_uniform_buffer(sizeof(GlobalUniformBuffer));
		if (!buffRes.has_value())
		{
			return false;
		}
		auto buff = buffRes.value();
		m_globalBuffers.push_back(buff);
		m_globalBufferMappedMem.push_back(buff->map_memory());
	}
	//X Create named global set
	{
		std::array<VkDescriptorSetLayoutBinding, 1> bindings;
		bindings[0].binding = 0;
		bindings[0].descriptorCount = 1;
		bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[0].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo setinfo = {};
		setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setinfo.pNext = nullptr;
		setinfo.bindingCount = bindings.size();
		setinfo.flags = 0;
		setinfo.pBindings = bindings.data();

		m_globalDataSetLayout = pipelineManager->create_or_get_named_set_layout("GlobalDataSetLayout", &setinfo);

		//X Create Draw Data Set Layout
		{
			//X Transform Buffer
			std::array<VkDescriptorSetLayoutBinding, 2> bindings;
			bindings[0].binding = 0;
			bindings[0].descriptorCount = 1;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[0].pImmutableSamplers = nullptr;

			//X Material Buffer
			bindings[1].binding = 1;
			bindings[1].descriptorCount = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[1].pImmutableSamplers = nullptr;


			VkDescriptorSetLayoutCreateInfo setinfo = {};
			setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			setinfo.pNext = nullptr;
			setinfo.bindingCount = bindings.size();
			setinfo.flags = 0;
			setinfo.pBindings = bindings.data();

			m_drawDataSetLayout = pipelineManager->create_or_get_named_set_layout("DrawDataSetLayout", &setinfo);
		}
		//X Create Light Set
		{
			//X Point Light Buffer
			std::array<VkDescriptorSetLayoutBinding, 1> bindings;
			bindings[0].binding = 0;
			bindings[0].descriptorCount = 1;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT |  VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[0].pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutCreateInfo setinfo = {};
			setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			setinfo.pNext = nullptr;
			setinfo.bindingCount = bindings.size();
			setinfo.flags = 0;
			setinfo.pBindings = bindings.data();

			m_globalLightSetLayout = pipelineManager->create_or_get_named_set_layout("LightSetLayout", &setinfo);
		}
	}

	//X Create the pool
	{
		//X First create necessary pool
		std::unordered_map<VkDescriptorType, int> map;
		map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
		map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3);

		//X 2 Global 1 Draw Data 1 Light Data
		m_globalPool = m_logicalDevice->create_and_init_vector_pool(map, m_framesInFlight+2);
	}

	//X Create set and update sets
	{
		std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight, m_globalDataSetLayout->get_layout());

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.pNext = nullptr;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = m_globalPool->get_vk_descriptor_pool();
		allocInfo.descriptorSetCount = layouts.size();
		allocInfo.pSetLayouts = layouts.data();
		m_globalSets.resize(m_framesInFlight);
		assert(VK_SUCCESS == vkAllocateDescriptorSets(m_logicalDevice->get_vk_device(), &allocInfo, m_globalSets.data()));

		auto vkSetLayout = m_drawDataSetLayout->get_layout();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &vkSetLayout;
		assert(VK_SUCCESS == vkAllocateDescriptorSets(m_logicalDevice->get_vk_device(), &allocInfo, &m_drawDataSet));
		
		vkSetLayout = m_globalLightSetLayout->get_layout();
		assert(VK_SUCCESS == vkAllocateDescriptorSets(m_logicalDevice->get_vk_device(), &allocInfo, &m_globalLightSet));

		//X Update sets
		VkDescriptorBufferInfo buff = {};
		buff.offset = 0;
		for (int i = 0; i < m_framesInFlight; i++)
		{
			buff.buffer = m_globalBuffers[i]->get_vk_buffer();
			buff.range = m_globalBuffers[i]->get_size();

			VkWriteDescriptorSet setWrite = {};
			setWrite = {};
			setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			setWrite.pNext = nullptr;
			setWrite.dstBinding = 0;
			setWrite.dstSet = m_globalSets[i];
			setWrite.descriptorCount = 1;
			setWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			setWrite.pBufferInfo = &buff;

			vkUpdateDescriptorSets(m_logicalDevice->get_vk_device(), 1, &setWrite, 0, nullptr);
		}
	}

	//X Create the renderer
	{
		m_deferredRenderer = new GSceneRenderer2(m_logicalDevice, pipelineManager,resManager,shaderManager,this,m_framesInFlight,VK_FORMAT_R8G8B8A8_UNORM);
		assert(m_deferredRenderer->init(m_globalDataSetLayout->get_layout(),m_drawDataSetLayout,m_globalLightSetLayout));	
		//X Create Global Draw Data
		{
			uint32_t countOfTranssformMatrix = m_deferredRenderer->get_max_count_of_draw_data();

			m_globalTransformData.gpuBuffer.reset(m_logicalDevice->create_buffer(countOfTranssformMatrix * sizeof(glm::mat4),
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
			m_globalTransformData.create_internals();

			m_globalMaterialData.gpuBuffer.reset(m_logicalDevice->create_buffer(GSceneRenderer2::calculate_nearest_10mb<MaterialDescription>() * sizeof(MaterialDescription),
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
			m_globalMaterialData.create_internals();

			m_globalPointLights.gpuBuffer.reset(m_logicalDevice->create_buffer(100 * sizeof(GPointLight),
				VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());	

			m_globalPointLights.create_internals();
		}
		//X Write Draw Data Sets Sets
		{
			//X Now update draw set
			{
				//X First Draw Data Set
				std::array<VkDescriptorBufferInfo, 2> infos;
				infos[0] = { .buffer = m_globalTransformData.gpuBuffer->get_vk_buffer() ,.offset = 0,.range = m_globalTransformData.gpuBuffer->get_size() };
				infos[1] = { .buffer = m_globalMaterialData.gpuBuffer->get_vk_buffer() ,.offset = 0,.range = m_globalMaterialData.gpuBuffer->get_size() };

				std::array<VkWriteDescriptorSet, 2> writeSets;
				writeSets[0] = {};
				writeSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeSets[0].descriptorCount = 1;
				writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				writeSets[0].dstBinding = 0;
				writeSets[0].dstSet = m_drawDataSet;
				writeSets[0].pBufferInfo = &infos[0];

				writeSets[1] = {};
				writeSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeSets[1].descriptorCount = 1;
				writeSets[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				writeSets[1].dstBinding = 1;
				writeSets[1].dstSet = m_drawDataSet;
				writeSets[1].pBufferInfo = &infos[1];

				vkUpdateDescriptorSets(m_logicalDevice->get_vk_device(), writeSets.size(), writeSets.data(), 0, 0);

				//X Update Light
				infos[0] = { .buffer = m_globalPointLights.gpuBuffer->get_vk_buffer() ,.offset = 0,.range = m_globalPointLights.gpuBuffer->get_size() };
				writeSets[0].dstSet = m_globalLightSet;

				vkUpdateDescriptorSets(m_logicalDevice->get_vk_device(), 1, writeSets.data(), 0, 0);

			}
		}
		m_deferredRenderer->set_drawdata_set(m_drawDataSet);
		m_deferredRenderer->set_lightdata_set(m_globalLightSet);
	}
	std::vector<MaterialDescription> desc;
	m_editorScene = Scene::create_scene_with_default_material(desc);
	m_currentScene = m_editorScene;
	add_materials_to_scene(&desc);
	return true;
}

void GSceneManager::destroy()
{
	for(int i = 0;i<m_globalBuffers.size();i++)
	{
		m_globalBuffers[i]->unmap_memory();
		m_globalBuffers[i]->unload();
		delete m_globalBuffers[i];
	}
	if (m_globalPool != nullptr)
	{
		m_globalPool->destroy();
		delete m_globalPool;
		m_globalPool = nullptr;
	}
	if (m_deferredRenderer != nullptr)
	{
		m_deferredRenderer->destroy();
		delete m_deferredRenderer;
		m_deferredRenderer = nullptr;
	}
	m_globalTransformData.destroy();
	m_globalMaterialData.destroy();
}

bool GSceneManager::init_deferred_renderer(IGVulkanNamedDeferredViewport* deferred)
{
	m_deferredTargetedViewport = deferred;
	m_deferredRenderer->set_composition_views(deferred->get_position_attachment(), deferred->get_albedo_attachment(),deferred->get_emission_attachment(),deferred->get_pbr_attachment(),deferred->get_sampler_for_named_attachment(""),m_deferredTargetedViewport,
		this->m_deferredTargetedViewport);
	return true;
}

bool GSceneManager::is_renderer_active()
{
	return m_deferredTargetedViewport != nullptr;
}

VkDescriptorSet_T* GSceneManager::get_global_set_for_frame(uint32_t frame) const noexcept
{
	return m_globalSets[frame];
}
IGVulkanDeferredRenderer* GSceneManager::get_deferred_renderer() const noexcept
{
	return m_deferredRenderer;
}

IGVulkanNamedDeferredViewport* GSceneManager::create_default_deferred_viewport(IGVulkanNamedRenderPass* deferredPass, IGVulkanNamedRenderPass* compositionPass, VkFormat compositionFormat)
{
	return new GVulkanNamedBaseDeferredViewport(m_logicalDevice,deferredPass,compositionPass,VK_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM,compositionFormat);
}

uint32_t GSceneManager::add_node_to_root()
{
	return Scene::add_node(*m_currentScene, 0, m_currentScene->hierarchy[0].level + 1);
}

uint32_t GSceneManager::add_mesh_to_scene(const MeshData* mesh)
{
	return this->m_deferredRenderer->add_mesh_to_scene(mesh);
}

uint32_t GSceneManager::add_node_with_mesh_and_defaults(uint32_t meshIndex)
{
	return add_node_with_mesh_and_material(meshIndex, 0);
}

Scene* GSceneManager::get_current_scene() const noexcept
{
	return m_currentScene;
}

std::span<MaterialDescription> GSceneManager::get_current_scene_materials()
{
	return std::span<MaterialDescription>(m_globalMaterialData.cpuVector.data(), m_globalMaterialData.inUsage);
}

uint32_t GSceneManager::register_texture_to_scene(IGTextureResource* textureRes)
{
	uint32_t currentIndex = m_inUsageTextures;
	m_inUsageTextures++;
	VkWriteDescriptorSet set = {};
	std::array<VkDescriptorImageInfo, 1> imageInfos;
	auto sampler = m_deferredTargetedViewport->get_sampler_for_named_attachment("");
	//it will be the camera buffer
	imageInfos[0].imageView = textureRes->get_vulkan_image()->get_vk_image_view();
	imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfos[0].sampler = sampler;

	std::array< VkWriteDescriptorSet, 1> setWrites;
	setWrites[0] = {};
	setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWrites[0].pNext = nullptr;
	setWrites[0].dstBinding = 0;
	setWrites[0].dstSet = m_deferredRenderer->get_bindless_set();
	setWrites[0].descriptorCount = 1;
	setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	setWrites[0].pImageInfo = &imageInfos[0];
	setWrites[0].dstArrayElement = currentIndex;

	vkUpdateDescriptorSets(m_logicalDevice->get_vk_device(), setWrites.size(), setWrites.data(), 0, nullptr);

	return currentIndex;
}

uint32_t GSceneManager::add_material_to_scene(const MaterialDescription* desc)
{
	std::vector<MaterialDescription> descs(1);
	descs[0] = *desc;
	return m_globalMaterialData.add_to_buffer(descs);
}

uint32_t GSceneManager::add_materials_to_scene(const std::vector<MaterialDescription>* desc)
{
	return m_globalMaterialData.add_to_buffer(*desc);
}

uint32_t GSceneManager::add_default_transform()
{

	std::vector<glm::mat4> transforms(1);
	transforms[0] = glm::identity<glm::mat4>();
	return m_globalTransformData.add_to_buffer(transforms);
}

void GSceneManager::set_transform_by_index(const glm::mat4* data, uint32_t gpuIndex)
{
	assert(gpuIndex < (m_globalTransformData.gpuBuffer->get_size() / sizeof(float) * 16));
	memcpy(&m_globalTransformData.gpuBegin[gpuIndex], glm::value_ptr(*data), sizeof(float) * 16);
}

uint32_t GSceneManager::add_node_with_mesh_and_material(uint32_t meshIndex, uint32_t materialIndex)
{
	//X First create a node
	uint32_t nodeId = add_node_to_root();

	//X First create transform data for the mesh
	uint32_t nodeGpuIndex = add_default_transform();
	m_cpu_to_gpu_map.emplace(nodeId, nodeGpuIndex);

	uint32_t drawId = m_deferredRenderer->create_draw_data(meshIndex, materialIndex, nodeGpuIndex);

	//X Add mesh and default material
	m_currentScene->meshes_.emplace(nodeId, meshIndex);
	m_currentScene->materialForNode_.emplace(nodeId, materialIndex);

	return nodeId;
}

uint32_t GSceneManager::add_node_with_mesh_and_material_and_transform(uint32_t meshIndex, uint32_t materialIndex, const glm::mat4* transform)
{
	//X First create a node
	uint32_t nodeId = add_node_to_root();

	//X First create transform data for the mesh
	uint32_t nodeGpuIndex = add_default_transform();
	
	m_currentScene->localTransform_[nodeId] = *transform;
	m_currentScene->mark_as_changed(nodeId);

	set_transform_by_index(transform, nodeGpuIndex);

	m_cpu_to_gpu_map.emplace(nodeId, nodeGpuIndex);

	uint32_t drawId = m_deferredRenderer->create_draw_data(meshIndex, materialIndex, nodeGpuIndex);

	//X Add mesh and default material
	m_currentScene->meshes_.emplace(nodeId, meshIndex);
	m_currentScene->materialForNode_.emplace(nodeId, materialIndex);

	return nodeId;
}

uint32_t GSceneManager::add_child_node_with_mesh_and_material_and_transform(uint32_t parentNode, uint32_t meshIndex, uint32_t materialIndex, const glm::mat4* transform)
{
	//X First create a node
	uint32_t nodeId = Scene::add_node(*m_currentScene, parentNode, m_currentScene->hierarchy[parentNode].level + 1);

	//X First create transform data for the mesh
	uint32_t nodeGpuIndex = add_default_transform();

	m_currentScene->localTransform_[nodeId] = *transform;
	m_currentScene->mark_as_changed(nodeId);

	set_transform_by_index(transform, nodeGpuIndex);

	m_cpu_to_gpu_map.emplace(nodeId, nodeGpuIndex);

	uint32_t drawId = m_deferredRenderer->create_draw_data(meshIndex, materialIndex, nodeGpuIndex);

	//X Add mesh and default material
	m_currentScene->meshes_.emplace(nodeId, meshIndex);
	m_currentScene->materialForNode_.emplace(nodeId, materialIndex);

	return nodeId;

}

uint32_t GSceneManager::add_point_light_node()
{
	std::vector<GPointLight> lights(1);
	lights[0] = GPointLight{};
	uint32_t lightIndex = m_globalPointLights.add_to_buffer(lights);
	m_globalData.pointLightCount++;
	uint32_t nodeId = add_node_to_root();
	m_nodeToLight.emplace(nodeId, lightIndex);
	return lightIndex;
}

void GSceneManager::set_material_by_index(const MaterialDescription* data,uint32_t gpuIndex)
{
	m_globalMaterialData.set_by_index(data, gpuIndex);
}
