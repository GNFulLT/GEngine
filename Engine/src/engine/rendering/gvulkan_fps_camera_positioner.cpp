#include "internal/engine/rendering/gvulkan_fps_camera_positioner.h"
#include "public/math/gcam.h"
#include "public/math/gtransform.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "public/core/templates/shared_ptr.h"
#include "public/platform/window.h"
#include "engine/manager/iglogger_manager.h"
#include "public/platform/ikeyboard_manager.h"

#include <algorithm>

GFpsCameraPositioner::GFpsCameraPositioner() : m_camDir(gvec3(0.f, 0.f, 1.f))
{
	m_cameraData.zNear = 0.1f;
	m_cameraData.zFar = 1000.f;
	m_projMatrix = perspective(70.f, 16.f / 9.f, 0.1f, 1000.f);
	m_viewMatrix = look_at(m_campos,m_campos+m_camDir,m_camUp);
	m_viewProjMatrix = m_projMatrix * m_viewMatrix;
	p_keyboardManager = nullptr;
	m_camSpeed = { 0,0,0 };

}

void GFpsCameraPositioner::update(float deltaTime)
{
	gvec3 accel = { 0,0,0 };
	
	if (p_keyboardManager->is_key_pressed(KEY_W))
		accel += (m_viewMatrix.forward);

	if (p_keyboardManager->is_key_pressed(KEY_S))
		accel -= (m_viewMatrix.forward);

	if (p_keyboardManager->is_key_pressed(KEY_D))
		accel -= (m_viewMatrix.left);

	if (p_keyboardManager->is_key_pressed(KEY_A))
		accel += (m_viewMatrix.left);
		
	if (m_camSpeed.len() != 0.f)
	{
		auto dampingC = std::min(1.f, (1.f / m_damping) * deltaTime);
		m_camSpeed -= m_camSpeed * dampingC;
	}

	bool isFast = false;
	if (p_keyboardManager->is_key_pressed(KEY_SHIFT))
		isFast = true;
	if (isFast)
		accel *= m_fastCoef;

	m_camSpeed += accel * m_acceleration * deltaTime;
	const float maxSpeed = isFast ? m_maxSpeed * m_fastCoef : m_maxSpeed;
	if (m_camSpeed.len() > maxSpeed)
	{
		m_camSpeed.normalize();
		m_camSpeed *= maxSpeed;
	}

	m_campos += m_camSpeed * deltaTime;

	//X Build the view matr

	m_viewMatrix = m_camOrientation.to_mat4() * translate(m_campos);
	m_viewProjMatrix = m_projMatrix * m_viewMatrix;

}

const float* GFpsCameraPositioner::get_position()
{
	return &m_campos.x;
}

bool GFpsCameraPositioner::init()
{
	p_keyboardManager = ((GSharedPtr<Window>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_WINDOW))->get()->get_keyboard_manager();
	return true;
}

const float* GFpsCameraPositioner::get_view_proj_matrix() const noexcept
{
	return &m_viewProjMatrix.xx;
}

const float* GFpsCameraPositioner::get_view_matrix() const noexcept
{
	return &m_viewMatrix.xx;
}

const float* GFpsCameraPositioner::get_proj_matrix() const noexcept
{
	return &m_projMatrix.xx;
}

const CameraData* GFpsCameraPositioner::get_camera_data() noexcept
{
	return &m_cameraData;
}

