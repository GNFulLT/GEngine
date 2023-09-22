#include "internal/engine/manager/gscene_manager.h"
#include "engine/gengine.h"
#include "engine/manager/igcamera_manager.h"
#include "engine/imanager_table.h"
#include "public/core/templates/shared_ptr.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"

IGVulkanUniformBuffer* GSceneManager::get_global_buffer_for_frame(uint32_t frame) const noexcept
{
	return m_globalBuffers[frame];
}

void GSceneManager::reconstruct_global_buffer_for_frame(uint32_t frame)
{
	//X Copy datas to gpu directly
	auto viewOffset = (void*)(std::uint64_t(m_globalBufferMappedMem[frame]) + (16 * sizeof(float)));
	auto pOffset = (void*)(std::uint64_t(viewOffset) + (3 * sizeof(float)));

	memcpy(m_globalBufferMappedMem[frame], m_cameraManager->get_camera_view_proj_matrix(), 16 * sizeof(float));
	memcpy(viewOffset, m_cameraManager->get_camera_position(), 3 * sizeof(float));
	memcpy(pOffset, m_cameraManager->get_camera_view_matrix(), 16 * sizeof(float));
}

bool GSceneManager::init(uint32_t framesInFlight)
{

	m_framesInFlight = framesInFlight;
	auto table = GEngine::get_instance()->get_manager_table();
	m_cameraManager = ((GSharedPtr<IGCameraManager>*)table->get_engine_manager_managed(ENGINE_MANAGER_CAMERA))->get();
	m_logicalDevice = ((GSharedPtr<IGVulkanDevice>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_GRAPHIC_DEVICE))->get()->as_logical_device().get();
	
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
}
