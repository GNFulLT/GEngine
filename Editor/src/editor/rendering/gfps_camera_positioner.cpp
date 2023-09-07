#include "internal/rendering/gfps_camera_positioner.h"
#include "public/math/gcam.h"
#include "public/math/gtransform.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "public/core/templates/shared_ptr.h"
#include "public/platform/window.h"
#include "engine/manager/iglogger_manager.h"
#include "public/platform/ikeyboard_manager.h"
#include "internal/imgui_window_manager.h"
#include <algorithm>
#include "public/platform/imouse_manager.h"
#include "editor/editor_application_impl.h"
#include <glm/gtx/euler_angles.hpp>

GEditorFPSCameraPositioner::GEditorFPSCameraPositioner(ImGuiWindowManager* windowManager) : m_camDir(gvec3(0.f, 0.f, -1.f))
{
	p_windowManager = windowManager;
	p_mouseManager = nullptr;
	m_projMatrix = perspective(70.f, 16.f / 9.f, 0.1f, 1000.f);
	m_viewMatrix =	look_at(m_campos,m_campos+m_camDir,m_camUp);
	m_camOrientation = mat4_to_quat(m_viewMatrix);
	m_viewProjMatrix = m_projMatrix * m_viewMatrix;
	p_keyboardManager = nullptr;
	m_camSpeed = { 0,0,0 };

}

void GEditorFPSCameraPositioner::update(float deltaTime)
{
	auto window = p_windowManager->get_window_if_exist(ImGuiWindowManager::VIEWPORT_NAME);
	if (window == nullptr || !window->is_focused())
		return;

	gvec3 accel = { 0,0,0 };
	gvec3 dir = { 0,0,0 };

	if(p_mouseManager->get_mouse_button_state(MOUSE_BUTTON_LEFT))
	{
		if (m_firstClick)
		{
			m_firstClick = false;
			m_mousePos = p_mouseManager->get_mouse_pos();
		}
		else
		{
			//X TODO : GVEC2
			std::pair<int, int> curr = p_mouseManager->get_mouse_pos();
			dir.x = -(curr.second - m_mousePos.second);
			dir.y = curr.first - m_mousePos.first;
			dir *= deltaTime;
			m_camOrientation = from_eular_angles(dir) * m_camOrientation;
			m_mousePos = curr;
			// Set up vector
			setup_up_vector();
		}
	}
	else
	{
		m_firstClick = true;
	}

	auto orientation = m_camOrientation.to_mat4();

	if (p_keyboardManager->is_key_pressed(KEY_W))
		accel += (orientation.forward);

	if (p_keyboardManager->is_key_pressed(KEY_S))
		accel -= (orientation.forward);

	if (p_keyboardManager->is_key_pressed(KEY_D))
		accel -= (orientation.left);

	if (p_keyboardManager->is_key_pressed(KEY_A))
		accel += (orientation.left);
		
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
	

}

const gmat4* GEditorFPSCameraPositioner::get_view_proj_projection()
{	
	m_viewMatrix = m_camOrientation.to_mat4() * translate(m_campos);
	m_viewProjMatrix = m_projMatrix * m_viewMatrix;
	return &m_viewProjMatrix;
}

const gvec3* GEditorFPSCameraPositioner::get_position()
{
	return &m_campos;
}

bool GEditorFPSCameraPositioner::init()
{
	auto window = ((GSharedPtr<Window>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_WINDOW))->get();
	p_keyboardManager = window->get_keyboard_manager();
	p_mouseManager = window->get_mouse_manager();
	return true;
}

void GEditorFPSCameraPositioner::setup_up_vector()
{/*
	m_viewMatrix = m_camOrientation * translate(m_campos);

	const glm::mat4 view = glm::mat4(m_viewMatrix.xx, m_viewMatrix.xy, m_viewMatrix.xz, m_viewMatrix.xw, m_viewMatrix.yx, m_viewMatrix.yy, m_viewMatrix.yz, m_viewMatrix.yw, m_viewMatrix.zx
		, m_viewMatrix.zy, m_viewMatrix.zz, m_viewMatrix.zw,m_viewMatrix.tx, m_viewMatrix.ty, m_viewMatrix.tz, m_viewMatrix.tw);
	const glm::vec3 dir = -glm::vec3(view[0][2], view[1][2], view[2][2]);
	auto cameraPosition_ = glm::vec3(m_campos.x, m_campos.y, m_campos.z);
	auto cameraOrientation_ = glm::lookAt(cameraPosition_, cameraPosition_ + dir, glm::vec3(0,1,0));
	m_camOrientation = gmat4(&cameraOrientation_.operator[](0).x);*/


	/*m_viewMatrix = m_camOrientation.to_mat4() * translate(m_campos);
	auto dir = gvec3(-m_viewMatrix.xx,-m_viewMatrix.yx,-m_viewMatrix.zx);
	auto t = look_at(m_campos,dir, m_camUp);
	m_camOrientation =	mat4_to_quat(t);

	m_viewMatrix = m_camOrientation * translate(m_campos);
	auto dir = gvec3(m_viewMatrix.xx, m_viewMatrix.yx, m_viewMatrix.zx);
	m_camOrientation = look_at(m_campos, dir, m_camUp);*/

	m_viewMatrix = m_camOrientation.to_mat4() * translate(m_campos);
	auto dir = gvec3(-m_viewMatrix.xx, -m_viewMatrix.yx, -m_viewMatrix.zx);
	m_viewMatrix = look_at(m_campos, m_campos + m_camDir, m_camUp);
	m_camOrientation = mat4_to_quat(m_viewMatrix);
}

const void* GEditorFPSCameraPositioner::get_matrix() const noexcept
{
	m_viewMatrix = m_camOrientation.to_mat4() * translate(m_campos);
	m_viewProjMatrix = m_projMatrix * m_viewMatrix;
	return &m_viewProjMatrix;;
}

