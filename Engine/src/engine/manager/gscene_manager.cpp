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
#include "engine/rendering/vulkan/ivulkan_queue.h"

struct AABB
{
	glm::vec4 min;
	glm::vec4 max;
};


//X 8x8
static const uint32_t TILE_SIZE = 8;
static const uint32_t BIN_SIZE = 16;

struct SortedLight {

	uint32_t             light_index;
	float             projected_z;
	float             projected_z_min;
	float             projected_z_max;
}; // struct SortedLight

static int sorting_light_fn(const void* a, const void* b) {
	const SortedLight* la = (const SortedLight*)a;
	const SortedLight* lb = (const SortedLight*)b;

	if (la->projected_z < lb->projected_z) return -1;
	else if (la->projected_z > lb->projected_z) return 1;
	return 0;
}

GSceneManager::GSceneManager()
{
	m_globalData.sunProperties.sunLightDirection[0] = 5.f;
	m_globalData.sunProperties.sunLightDirection[1] = 30.f;
	m_globalData.sunProperties.sunLightDirection[2] = 4.f;
	m_globalData.sunProperties.sunLightColor[0] = 1.f;
	m_globalData.sunProperties.sunLightColor[1] = 1.f;
	m_globalData.sunProperties.sunLightColor[2] = 1.f;

}

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
	m_globalData.resolution[1] = std::abs(m_deferredTargetedViewport->get_viewport_area()->height);
	
	m_globalData.zNear = m_cameraManager->get_camera_data()->zNear;
	m_globalData.zFar = 96.f;

	auto lightPos = glm::normalize(glm::make_vec3(m_globalData.sunProperties.sunLightDirection)) * 40.f;
	//glm::mat4 depthProjectionMatrix = glm::ortho(-35.f, 35.f,-35.f,35.f, globalData->zNear, globalData->zFar);
	glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(45.f), 1.f, m_globalData.zNear, m_globalData.zFar);

	glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));

	auto lp = depthProjectionMatrix * depthViewMatrix * glm::mat4(1.f);
	memcpy(m_globalData.sunLP, glm::value_ptr(lp),sizeof(glm::mat4));
	//X Copy data to gpu
	memcpy(m_globalBufferMappedMem[frame], &m_globalData,sizeof(GlobalUniformBuffer));
	
	//X Update Cull Data
	m_globalCullData.cullData = *m_cameraManager->get_cull_data();
	m_globalCullData.cullData.drawCount = m_deferredRenderer->get_max_indirect_draw_count();
	memcpy(m_globalCullBufferMappedMems[frame], &m_globalCullData, sizeof(GlobalCullBuffer));
	
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
	std::vector<SortedLight> sortedLights(m_globalData.pointLightCount);
	//X Set to frustum far plane
	float zFar = 10.f;
	//X Iterate the point lights
	glm::mat4 camView = glm::make_mat4(m_cameraManager->get_camera_view_matrix());

	for (int i = 0; i < m_globalData.pointLightCount; i++)
	{
		GPointLight& light = m_globalPointLights.cpuVector[i];
		glm::vec4 p = glm::vec4(light.position, 1.f);
	
		auto projected_p = camView * p;
		auto projected_p_min = projected_p + glm::vec4(0, 0, light.radius, 0.f);
		auto projected_p_max = projected_p + glm::vec4(0, 0, -light.radius, 0.f);

		SortedLight& sortedLight = sortedLights[i];
		sortedLight.light_index = i;
		sortedLight.projected_z = (m_cameraManager->get_camera_data()->zNear - projected_p.z) / (zFar - m_cameraManager->get_camera_data()->zNear);
		sortedLight.projected_z_min = (m_cameraManager->get_camera_data()->zNear - projected_p_min.z) / (zFar - m_cameraManager->get_camera_data()->zNear);
		sortedLight.projected_z_max = (m_cameraManager->get_camera_data()->zNear - projected_p_max.z) / (zFar - m_cameraManager->get_camera_data()->zNear);
	}
	qsort(sortedLights.data(), m_globalData.pointLightCount, sizeof(SortedLight), sorting_light_fn);

	for (int i = 0; i < m_globalData.pointLightCount; i++)
	{
		SortedLight& sortedLight = sortedLights[i];
		m_globalPointLightIndices[frame].cpuVector[i] = sortedLight.light_index;
	}
	memcpy(m_globalPointLightIndices[frame].gpuBegin, m_globalPointLightIndices[frame].cpuVector.data(),sizeof(uint32_t)* m_globalData.pointLightCount);
	
	//x Create bins per light

	const float binSize = 1.0f / BIN_SIZE;
	std::vector<uint32_t> binRangePerLight(m_globalData.pointLightCount);
	for (int i = 0; i < m_globalData.pointLightCount; i++)
	{
		const SortedLight& sortedLight = sortedLights[i];

		if (sortedLight.projected_z_min < 0.0f && sortedLight.projected_z_max < 0.0f) {
			// Light is behind the camera
			binRangePerLight[i] = UINT32_MAX;
			continue;
		}
		const uint32_t min_bin = glm::max(0.f, glm::floor(sortedLight.projected_z_min * BIN_SIZE));
		const uint32_t max_bin = glm::max(0.f, glm::ceil(sortedLight.projected_z_max * BIN_SIZE));

		binRangePerLight[i] = (min_bin & 0xffff) | ((max_bin & 0xffff) << 16);
	}

	std::vector<uint32_t> bins(BIN_SIZE);
	//X Set Bins
	for (int i = 0; i < BIN_SIZE; i++)
	{
		uint32_t minLightId = m_globalData.pointLightCount + 1;
		uint32_t maxLightId = 0;
			
		for (int j = 0; j < m_globalData.pointLightCount; j++)
		{
			const SortedLight& sortedLight = sortedLights[j];
			const uint32_t binsOfLight = binRangePerLight[j];
			if (binsOfLight == UINT32_MAX)
				continue;

			const uint32_t min_bin = binsOfLight & 0xffff;
			const uint32_t max_bin = binsOfLight >> 16;

			if (i >= min_bin && i <= max_bin) {
				if (j < minLightId) {
					minLightId = j;
				}

				if (j > maxLightId) {
					maxLightId = j;
				}
			}
		}

		bins[i] = minLightId | (maxLightId << 16);
	}
	memcpy(m_globalPointLightBins[frame].gpuBegin,bins.data(),sizeof(uint32_t) * BIN_SIZE);
	uint32_t minLightIdBin = bins[0] & 0XFFFF;
	uint32_t maxLightIdBin = (bins[0] >> 16) & 0XFFFF;

	const uint32_t tileXCount = m_globalData.resolution[0] / TILE_SIZE;
	const uint32_t tileYCount = m_globalData.resolution[1] / TILE_SIZE;
	const uint32_t numWords = (m_globalData.pointLightCount + 31) / 32;
	const uint32_t tilesEntryCount = tileXCount * tileYCount * numWords;
	const uint32_t bufferSize = tilesEntryCount * sizeof(uint32_t);
	float tileSizeInv = 1.0f / TILE_SIZE;

	uint32_t tileStride = tileXCount * numWords;

	std::vector<uint32_t> tilesBit(tilesEntryCount,0);
	auto zNear = m_cameraManager->get_camera_data()->zNear;
	for (int i = 0; i < m_globalData.pointLightCount; i++)
	{
		GPointLight& light = m_globalPointLights.cpuVector[sortedLights[i].light_index];


		glm::vec4 pos = glm::vec4(light.position, 1.f);
		float radius = light.radius;
		glm::vec4 viewSpacePos = camView * pos;
		bool cameraVisible = -viewSpacePos.z - radius < m_cameraManager->get_camera_data()->zNear;
		if (!cameraVisible)
		{
			continue;
		}

		//X Build an AABB for this light
		glm::vec4 aabb_min(FLT_MAX,FLT_MAX,FLT_MAX,FLT_MAX);
		glm::vec4 aabb_max(FLT_MIN, FLT_MIN, FLT_MIN, FLT_MIN);
		glm::vec4 aabb;
		for (int c = 0; c < 8; c++)
		{
			glm::vec3 corner((c % 2) ? 1.f : -1.f, (c & 2) ? 1.f : -1.f, (c & 4) ? 1.f : -1.f);
			corner = corner * radius;
			corner = corner + glm::vec3(pos);

			glm::vec4 viewSpaceCorner = camView * glm::vec4(corner,1.f);

			//X Clamp the coordinates
			viewSpaceCorner.z = glm::max(zNear, viewSpaceCorner.z);

			glm::vec4 viewProjCorner = camView * viewSpaceCorner;
			viewProjCorner = viewProjCorner / viewProjCorner.w;
			aabb_min.x = glm::min(aabb_min.x, viewProjCorner.x);
			aabb_min.y = glm::min(aabb_min.y, viewProjCorner.y);

			aabb_max.x = glm::max(aabb_max.x, viewProjCorner.x);
			aabb_max.y = glm::max(aabb_max.y, viewProjCorner.y);
		}
		aabb.x = aabb_min.x;
		aabb.z = aabb_max.x;
		// Invert aabb
		aabb.w = -1 * aabb_min.y;
		aabb.y = -1 * aabb_max.y;

		glm::vec4 aabb_screen{ (aabb.x * 0.5f + 0.5f) * (m_globalData.resolution[0] - 1),
						 (aabb.y * 0.5f + 0.5f) * (m_globalData.resolution[1] - 1),
						 (aabb.z * 0.5f + 0.5f) * (m_globalData.resolution[0] - 1),
						 (aabb.w * 0.5f + 0.5f) * (m_globalData.resolution[1] - 1) };

		float width = aabb_screen.z - aabb_screen.x;
		float height = aabb_screen.w - aabb_screen.y;

		if (width < 0.0001f || height < 0.0001f) {
			continue;
		}
		float min_x = aabb_screen.x;
		float min_y = aabb_screen.y;

		float max_x = min_x + width;
		float max_y = min_y + height;

		if (min_x > m_globalData.resolution[0] || min_y > m_globalData.resolution[1]) {
			continue;
		}

		if (max_x < 0.0f || max_y < 0.0f) {
			continue;
		}

		min_x = glm::max(min_x, 0.0f);
		min_y = glm::max(min_y, 0.0f);

		max_x = glm::min(max_x, (m_globalData.resolution[0]));
		max_y = glm::min(max_y, (m_globalData.resolution[1]));

		uint32_t first_tile_x = (uint32_t)(min_x * tileSizeInv);
		uint32_t last_tile_x = glm::min(tileXCount - 1, (uint32_t)(max_x * tileSizeInv));

		uint32_t first_tile_y = (uint32_t)(min_y * tileSizeInv);
		uint32_t last_tile_y = glm::min(tileYCount - 1, (uint32_t)(max_y * tileSizeInv));

		for (uint32_t y = first_tile_y; y <= last_tile_y; ++y) {
			for (uint32_t x = first_tile_x; x <= last_tile_x; ++x) {
				uint32_t array_index = y * tileStride + x;

				uint32_t word_index = i / 32;
				uint32_t bit_index = i % 32;

				tilesBit[array_index + word_index] |= (1 << bit_index);
			}
		}
	}

	//X Copy tiles bit
	memcpy(m_globalPointLightTiles[frame].gpuBegin,tilesBit.data(),tilesBit.size() * sizeof(uint32_t));
	m_deferredRenderer->set_lightdata_set(m_globalLightSets[frame]);
	m_deferredRenderer->set_culldata_set(m_cullDataSets[frame]);

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

	m_gpuCpuDataBuffers.resize(m_framesInFlight);
	m_gpuCpuDataBufferMappedMems.resize(m_framesInFlight);
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
		auto cullBuff = m_logicalDevice->create_buffer(sizeof(GlobalCullBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU).value();
		m_globalCullBuffers.push_back(cullBuff);
		m_globalCullBufferMappedMems.push_back(cullBuff->map_memory());
		m_gpuCpuDataBuffers[i].reset(m_logicalDevice->create_buffer(sizeof(GPUCPUData),VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VMA_MEMORY_USAGE_GPU_TO_CPU).value());
		m_gpuCpuDataBufferMappedMems[i] = m_gpuCpuDataBuffers[i]->map_memory();
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
		//X Create Cull Data Set Layout
		{
			std::array<VkDescriptorSetLayoutBinding, 1> bindings;
			bindings[0].binding = 0;
			bindings[0].descriptorCount = 1;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[0].pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutCreateInfo setinfo = {};
			setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			setinfo.pNext = nullptr;
			setinfo.bindingCount = bindings.size();
			setinfo.flags = 0;
			setinfo.pBindings = bindings.data();

			m_cullDataLayout = pipelineManager->create_or_get_named_set_layout("CullDataSetLayout", &setinfo);
		}
		//X Create GPU CPU Set Layout
		{
			std::array<VkDescriptorSetLayoutBinding, 1> bindings;
			bindings[0].binding = 0;
			bindings[0].descriptorCount = 1;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			bindings[0].stageFlags = VK_SHADER_STAGE_ALL;
			bindings[0].pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutCreateInfo setinfo = {};
			setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			setinfo.pNext = nullptr;
			setinfo.bindingCount = bindings.size();
			setinfo.flags = 0;
			setinfo.pBindings = bindings.data();

			m_gpuCpuDataSetLayout = pipelineManager->create_or_get_named_set_layout("GPUCPUDataSetLayout", &setinfo);
		}
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
			std::array<VkDescriptorSetLayoutBinding, 4> bindings;
			bindings[0].binding = 0;
			bindings[0].descriptorCount = 1;
			bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT |  VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[0].pImmutableSamplers = nullptr;

			bindings[1].binding = 1;
			bindings[1].descriptorCount = 1;
			bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[1].pImmutableSamplers = nullptr;

			bindings[2].binding = 2;
			bindings[2].descriptorCount = 1;
			bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[2].pImmutableSamplers = nullptr;


			bindings[3].binding = 3;
			bindings[3].descriptorCount = 1;
			bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
			bindings[3].pImmutableSamplers = nullptr;

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
		map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
		map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 6); 
		//X Cull Data should be as frames 
		//X FRAMES Global 1 Draw Data FRAMES Light Data + FRAMES Cull Data
		m_globalPool = m_logicalDevice->create_and_init_vector_pool(map, (m_framesInFlight*3)+1);
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

		//X Create drawdataset 
		std::vector<VkDescriptorSetLayout> lightSetLayouts(framesInFlight, m_globalLightSetLayout->get_layout());
		allocInfo.descriptorSetCount = lightSetLayouts.size();
		allocInfo.pSetLayouts = lightSetLayouts.data();
		m_globalLightSets.resize(lightSetLayouts.size());
		assert(VK_SUCCESS == vkAllocateDescriptorSets(m_logicalDevice->get_vk_device(), &allocInfo, m_globalLightSets.data()));

		std::vector<VkDescriptorSetLayout> cullSetLayout(m_framesInFlight, m_cullDataLayout->get_layout());
		allocInfo.pSetLayouts = cullSetLayout.data();
		allocInfo.descriptorSetCount = cullSetLayout.size();
		m_cullDataSets.resize(m_framesInFlight);

		assert(VK_SUCCESS == vkAllocateDescriptorSets(m_logicalDevice->get_vk_device(), &allocInfo, m_cullDataSets.data()));

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

			//X Write Sets
			{
				//X First Draw Data Set
				std::array<VkDescriptorBufferInfo, 1> infos;
				std::array<VkWriteDescriptorSet, 1> writeSets;

				//X Now Write Cull data
				infos[0] = { .buffer = m_globalCullBuffers[i]->get_vk_buffer() ,.offset = 0,.range = m_globalCullBuffers[i]->get_size()};
				writeSets[0] = {};
				writeSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeSets[0].descriptorCount = 1;
				writeSets[0].dstBinding = 0;
				writeSets[0].pBufferInfo = &infos[0];
				writeSets[0].dstSet = m_cullDataSets[i];
				writeSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

				vkUpdateDescriptorSets(m_logicalDevice->get_vk_device(), 1, writeSets.data(), 0, 0);

			}
		}


	}

	//X Create the renderer
	{
		m_deferredRenderer = new GSceneRenderer2(m_logicalDevice, pipelineManager,resManager,shaderManager,this,m_framesInFlight,VK_FORMAT_R8G8B8A8_UNORM);
		assert(m_deferredRenderer->init(m_globalDataSetLayout->get_layout(),m_drawDataSetLayout,m_globalLightSetLayout, m_cullDataLayout));
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
			m_globalPointLightIndices.resize(framesInFlight);
			m_globalPointLightTiles.resize(framesInFlight);
			m_globalPointLightBins.resize(framesInFlight);
			for (int i = 0; i < framesInFlight; i++)
			{
				m_globalPointLightIndices[i].gpuBuffer.reset(m_logicalDevice->create_buffer(100 * sizeof(uint32_t),
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
				m_globalPointLightIndices[i].create_internals();
				
				m_globalPointLightBins[i].gpuBuffer.reset(m_logicalDevice->create_buffer(100 * sizeof(uint32_t),
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
				m_globalPointLightBins[i].create_internals();

				m_globalPointLightTiles[i].gpuBuffer.reset(m_logicalDevice->create_buffer(1920 * sizeof(uint32_t) * 135,
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU).value());
				m_globalPointLightTiles[i].create_internals();
				
			}
		
			
		}
		//X Write Draw Data Sets Sets
		{
			//X Now update draw set
			{
				//X First Draw Data Set
				std::array<VkDescriptorBufferInfo, 4> infos;
				infos[0] = { .buffer = m_globalTransformData.gpuBuffer->get_vk_buffer() ,.offset = 0,.range = m_globalTransformData.gpuBuffer->get_size() };
				infos[1] = { .buffer = m_globalMaterialData.gpuBuffer->get_vk_buffer() ,.offset = 0,.range = m_globalMaterialData.gpuBuffer->get_size() };
				
				std::array<VkWriteDescriptorSet, 4> writeSets;
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

				vkUpdateDescriptorSets(m_logicalDevice->get_vk_device(), 2, writeSets.data(), 0, 0);

				writeSets[2] = {};
				writeSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeSets[2].descriptorCount = 1;
				writeSets[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				writeSets[2].dstBinding = 2;
				writeSets[2].pBufferInfo = &infos[2];

				writeSets[3] = {};
				writeSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeSets[3].descriptorCount = 1;
				writeSets[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				writeSets[3].dstBinding = 3;
				writeSets[3].pBufferInfo = &infos[3];

				//X Update Light
				infos[0] = { .buffer = m_globalPointLights.gpuBuffer->get_vk_buffer() ,.offset = 0,.range = m_globalPointLights.gpuBuffer->get_size() };
							
				for (int i = 0; i < m_framesInFlight; i++)
				{
					infos[1] = { .buffer = m_globalPointLightIndices[i].gpuBuffer->get_vk_buffer() ,.offset = 0,.range = m_globalPointLightIndices[i].gpuBuffer->get_size()};
					infos[2] = { .buffer = m_globalPointLightTiles[i].gpuBuffer->get_vk_buffer() ,.offset = 0,.range = m_globalPointLightTiles[i].gpuBuffer->get_size()};
					infos[3] = { .buffer = m_globalPointLightBins[i].gpuBuffer->get_vk_buffer() ,.offset = 0,.range = m_globalPointLightBins[i].gpuBuffer->get_size() };

					writeSets[0].dstSet = m_globalLightSets[i];
					writeSets[1].dstSet = m_globalLightSets[i];
					writeSets[2].dstSet = m_globalLightSets[i];
					writeSets[3].dstSet = m_globalLightSets[i];

					vkUpdateDescriptorSets(m_logicalDevice->get_vk_device(), 4, writeSets.data(), 0, 0);
				}


			}
		}
		m_deferredRenderer->set_drawdata_set(m_drawDataSet);
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
		m_globalCullBuffers[i]->unmap_memory();
		m_globalCullBuffers[i]->unload();
		delete m_globalCullBuffers[i];
		m_gpuCpuDataBuffers[i]->unmap_memory();
		m_gpuCpuDataBuffers[i]->unload();
		m_gpuCpuDataBuffers[i].reset();
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
	m_globalPointLights.destroy();
	for (int i = 0; i < m_framesInFlight; i++)
	{
		m_globalPointLightBins[i].destroy();
		m_globalPointLightTiles[i].destroy();
		m_globalPointLightIndices[i].destroy();
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
	m_registeredTextureMap.emplace(currentIndex,textureRes);
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
	m_nodeToDrawID.emplace(nodeId, drawId);
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
	m_nodeToDrawID.emplace(nodeId, drawId);
	return nodeId;

}

bool GSceneManager::is_cull_enabled()
{
	return m_globalCullData.cullEnabled;
}

void GSceneManager::set_cull_enabled(bool cullEnabled)
{
	m_globalCullData.cullEnabled = cullEnabled;
}

uint32_t GSceneManager::get_draw_id_of_node(uint32_t nodeId)
{
	if (auto iter = m_nodeToDrawID.find(nodeId); iter != m_nodeToDrawID.end())
	{
		return iter->second;
	}
	// UINT32_MAX
	return -1;
}

uint32_t GSceneManager::get_gpu_transform_index(uint32_t nodeId) const noexcept
{
	if (auto g = m_cpu_to_gpu_map.find(nodeId); g != m_cpu_to_gpu_map.end())
	{
		return g->second;
	}
	return -1;
}

const DrawData* GSceneManager::get_draw_data_by_id(uint32_t drawId) const noexcept
{
	return m_deferredRenderer->get_draw_data_by_id(drawId);
}

IGTextureResource* GSceneManager::get_saved_texture_by_id(uint32_t textureId) const noexcept
{
	if (auto textureIter = m_registeredTextureMap.find(textureId); textureIter != m_registeredTextureMap.end())
	{
		return textureIter->second;
	}
	return nullptr;
}

bool GSceneManager::is_node_light(uint32_t nodeId) const noexcept
{
	auto iter = m_nodeToLight.find(nodeId);
	return iter != m_nodeToLight.end();
}

const GPointLight* GSceneManager::get_point_light(uint32_t nodeId) const noexcept
{
	if (auto iter = m_nodeToLight.find(nodeId); iter != m_nodeToLight.end())
	{
		return &m_globalPointLights.cpuVector[iter->second];
	}
	return nullptr;
}

void GSceneManager::set_point_light(const GPointLight* data, uint32_t nodeId) noexcept
{
	if (auto iter = m_nodeToLight.find(nodeId); iter != m_nodeToLight.end())
	{
		m_globalPointLights.set_by_index(data, iter->second);
	}
}

const SunProperties* GSceneManager::get_sun_properties() const noexcept
{
	return &m_globalData.sunProperties;
}

void GSceneManager::update_sun_properties(const SunProperties* sunProps)
{
	m_globalData.sunProperties = *sunProps;
}

const GlobalUniformBuffer* GSceneManager::get_global_data() const noexcept
{
	return &m_globalData;
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
