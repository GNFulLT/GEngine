#include "internal/engine/manager/gcamera_manager.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "engine/rendering/vulkan/ivulkan_device.h"
#include "public/core/templates/shared_ptr.h"
#include "public/math/gcam.h"
#include "internal/engine/rendering/gvulkan_fps_camera_positioner.h"
#include <glm/matrix.hpp>
#include <glm/ext.hpp>

void getFrustumPlanes(glm::mat4 mvp, glm::vec4* planes)
{
	using glm::vec4;

	mvp = glm::transpose(mvp);
	planes[0] = vec4(mvp[3] + mvp[0]); // left
	planes[1] = vec4(mvp[3] - mvp[0]); // right
	planes[2] = vec4(mvp[3] + mvp[1]); // bottom
	planes[3] = vec4(mvp[3] - mvp[1]); // top
	planes[4] = vec4(mvp[3] + mvp[2]); // near
	planes[5] = vec4(mvp[3] - mvp[2]); // far
}

void getFrustumCorners(glm::mat4 mvp, glm::vec4* points)
{
	using glm::vec4;

	const vec4 corners[] = {
		vec4(-1, -1, -1, 1), vec4(1, -1, -1, 1),
		vec4(1,  1, -1, 1),  vec4(-1,  1, -1, 1),
		vec4(-1, -1,  1, 1), vec4(1, -1,  1, 1),
		vec4(1,  1,  1, 1),  vec4(-1,  1,  1, 1)
	};

	const glm::mat4 invMVP = glm::inverse(mvp);

	for (int i = 0; i != 8; i++) {
		const vec4 q = invMVP * corners[i];
		points[i] = q / q.w;
	}
}


GCameraManager::GCameraManager(uint32_t framesInFlight)
{
	m_defaultPositioner = new GFpsCameraPositioner();
	m_selectedPositioner = m_defaultPositioner;
	m_framesInFlight = framesInFlight;
	m_boundedDevice = nullptr;
	m_cullData = {};
	m_frustrumProjMatrix = glm::perspective(70.f, 16.f / 9.f, 0.001f, 150.f);
	m_cullData.lodBase = 50.f;
	m_cullData.lodStep = 1.5f;
}

const float* GCameraManager::get_camera_position()
{
	return m_selectedPositioner->get_position();
}

bool GCameraManager::init()
{
	auto ldev = ((GSharedPtr<IGVulkanDevice>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_GRAPHIC_DEVICE))->get()->as_logical_device().get();
	for (int i = 0; i < m_framesInFlight; i++)
	{
		auto buffRes = ldev->create_uniform_buffer(sizeof(gmat4));
		if (!buffRes.has_value())
		{
			return false;
		}


		auto buff = buffRes.value();
		m_uniformBuffers.push_back(buff);
	}

	s_static = this;

	m_defaultPositioner->init();

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
	//X Update frustrum
	m_selectedPositioner->update(deltaTime);

	auto vp = m_frustrumProjMatrix * glm::make_mat4(m_selectedPositioner->get_view_matrix());

	getFrustumPlanes(vp, m_cullData.frustumPlanes);
	getFrustumCorners(vp, m_cullData.frustumCorners);
}

ICameraPositioner* GCameraManager::get_current_positioner()
{
	return m_selectedPositioner;
}

void GCameraManager::set_positioner(ICameraPositioner* positioner)
{
	//X ATTACH AND DETACH FUNCS
	bool inited = positioner->init();
	if (inited)
	{
		m_selectedPositioner = positioner;
	}
}

const float* GCameraManager::get_camera_view_matrix()
{
	return m_selectedPositioner->get_view_matrix();
}

const float* GCameraManager::get_camera_proj_matrix()
{
	return m_selectedPositioner->get_proj_matrix();
}

const float* GCameraManager::get_camera_view_proj_matrix()
{
	return m_selectedPositioner->get_view_proj_matrix();
}

const DrawCullData* GCameraManager::get_cull_data() const noexcept
{
	return &m_cullData;
}

void GCameraManager::update_frustrum_proj_matrix(const glm::mat4& frustrum) noexcept
{
	m_frustrumProjMatrix = frustrum;
}

DrawCullData* GCameraManager::get_cull_data() noexcept
{
	return &m_cullData;
}
