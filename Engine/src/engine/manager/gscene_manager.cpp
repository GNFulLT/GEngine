#include "internal/engine/manager/gscene_manager.h"
#include "engine/gengine.h"
#include "engine/manager/igcamera_manager.h"
#include "engine/imanager_table.h"
#include "public/core/templates/shared_ptr.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"
#include <array>
#include "internal/engine/rendering/vulkan/named/viewports/gvulkan_named_base_deferred_viewport.h"

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
		bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
		bindings[0].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo setinfo = {};
		setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setinfo.pNext = nullptr;
		setinfo.bindingCount = bindings.size();
		setinfo.flags = 0;
		setinfo.pBindings = bindings.data();

		m_globalDataSetLayout = pipelineManager->create_or_get_named_set_layout("GlobalDataSetLayout", &setinfo);
	}

	//X Create the pool
	{
		//X First create necessary pool
		std::unordered_map<VkDescriptorType, int> map;
		map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);

		m_globalPool = m_logicalDevice->create_and_init_vector_pool(map, m_framesInFlight);
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
		assert(m_deferredRenderer->init(m_globalDataSetLayout->get_layout()));
	}
	std::vector<MaterialDescription> desc;
	m_editorScene = Scene::create_scene_with_default_material(desc);
	m_currentScene = m_editorScene;
	m_deferredRenderer->add_material_to_scene(desc);
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
	//X First create a node
	uint32_t nodeId = add_node_to_root();

	//X First create transform data for the mesh
	uint32_t nodeGpuIndex = m_deferredRenderer->add_default_transform();
	m_cpu_to_gpu_map.emplace(nodeId, nodeGpuIndex);

	uint32_t drawId = m_deferredRenderer->create_draw_data(meshIndex, 0, nodeGpuIndex);

	//X Add mesh and default material
	m_currentScene->meshes_.emplace(nodeId, meshIndex);
	m_currentScene->materialForNode_.emplace(nodeId, 0);
	
	return nodeId;
}
