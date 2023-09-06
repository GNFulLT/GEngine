#include "internal/engine/manager/gcamera_manager.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "public/core/templates/shared_ptr.h"
#include "public/math/gcam.h"
#include "internal/engine/rendering/gvulkan_fps_camera_positioner.h"

GCameraManager::GCameraManager(uint32_t framesInFlight)
{
	m_defaultPositioner = new GFpsCameraPositioner();
	m_selectedPositioner = m_defaultPositioner;
	m_framesInFlight = framesInFlight;
}

bool GCameraManager::init()
{
	auto ldev = ((GSharedPtr<IGVulkanDevice>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_GRAPHIC_DEVICE))->get()->as_logical_device().get();
	//for (int i = 0; i < m_framesInFlight; i++)
	//{
	//	auto buffRes = ldev->create_uniform_buffer(sizeof(gmat4));
	//	if (!buffRes.has_value())
	//	{
	//		return false;
	//	}


	//	auto buff = buffRes.value();
	//	m_uniformBuffers.push_back(buff);
	//}
	//
	//auto buffRes = ldev->create_uniform_buffer(sizeof(gmat4));
	//if (!buffRes.has_value())
	//{
	//	return false;
	//}

	//auto buff = buffRes.value();
	//m_uniformBuffers.push_back(buff);
	s_static = this;

	return true;
}

void GCameraManager::destroy()
{
	if (m_defaultPositioner != nullptr)
	{
		delete m_defaultPositioner;
	}
	for (int i = 0; i < m_uniformBuffers.size(); i++)
	{
		m_uniformBuffers[i]->unload();
		delete m_uniformBuffers[i];
	}
}

void GCameraManager::update(float deltaTime)
{
	m_selectedPositioner->update(deltaTime);
	
	
}

ICameraPositioner* GCameraManager::get_current_positioner()
{
	return m_selectedPositioner;
}

const std::vector<IGVulkanUniformBuffer*>* GCameraManager::get_buffers()
{
	return &m_uniformBuffers;
}

void GCameraManager::render(uint32_t frame)
{
	auto viewProj = m_selectedPositioner->get_view_proj_projection();
	m_uniformBuffers[frame]->copy_data_to_device_memory(viewProj, sizeof(gmat4));
}
