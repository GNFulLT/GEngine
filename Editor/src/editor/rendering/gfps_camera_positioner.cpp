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
#include <glm/gtc/type_ptr.hpp>

GEditorFPSCameraPositioner::GEditorFPSCameraPositioner(ImGuiWindowManager* windowManager) 
{
	p_windowManager = windowManager;
	p_mouseManager = nullptr;
	m_cameraData.zNear = 0.001f;
	m_cameraData.zFar = 1000.f;
	m_projection = glm::perspective(70.f, 16.f / 9.f, 0.001f, 1000.f);
	cameraPosition_ = glm::vec3(1, 0, -5.f);
	up_ = glm::vec3(0,1,0);
	cameraOrientation_ = glm::lookAt(cameraPosition_,glm::vec3(0,0,-1), up_);
	m_viewProj = glm::mat4(1.f);
	m_view = glm::mat4(1.f);

	const glm::mat4 t = glm::translate(glm::mat4(1.0f), -cameraPosition_);
	const glm::mat4 r = glm::mat4_cast(cameraOrientation_);
	m_view = r * t;
	m_viewProj = m_projection * m_view;
}

const float* GEditorFPSCameraPositioner::get_view_proj_matrix() const noexcept
{
	return glm::value_ptr(m_viewProj);
}

const float* GEditorFPSCameraPositioner::get_view_matrix() const noexcept
{
	return glm::value_ptr(m_view);
}

const float* GEditorFPSCameraPositioner::get_proj_matrix() const noexcept
{
	return glm::value_ptr(m_projection);
}

void GEditorFPSCameraPositioner::update(float deltaTime)
{
	auto window = p_windowManager->get_window_if_exist("Composition");
	if (window == nullptr || !window->is_focused())
	{
		m_mousePos = p_mouseManager->get_mouse_pos();
		return;
	}


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
			glm::quat delta = glm::quat( glm::vec3(((curr.second - m_mousePos.second))* mouseSpeed_, (curr.first - m_mousePos.first)* mouseSpeed_, 0));
			cameraOrientation_ = delta * cameraOrientation_;
			cameraOrientation_ = glm::normalize(cameraOrientation_);
			m_mousePos = curr;
			// Set up vector
			setup_up_vector();
		}
	}
	else
	{
		m_firstClick = true;
	}

	const glm::mat4 v = glm::mat4_cast(cameraOrientation_);

	const glm::vec3 forward = -glm::vec3(v[0][2], v[1][2], v[2][2]);
	const glm::vec3 right = glm::vec3(v[0][0], v[1][0], v[2][0]);
	const glm::vec3 up = glm::cross(right, forward);

	glm::vec3 accel = { 0,0,0 };

	if (p_keyboardManager->is_key_pressed(KEY_W))
		accel += (forward);

	if (p_keyboardManager->is_key_pressed(KEY_S))
		accel -= (forward);

	if (p_keyboardManager->is_key_pressed(KEY_D))
		accel += (right);

	if (p_keyboardManager->is_key_pressed(KEY_A))
		accel -= (right);
		
	if (moveSpeed_.length() != 0.f)
	{
		auto dampingC = std::min(1.f, (1.f / m_damping) * deltaTime);
		moveSpeed_ -= moveSpeed_ * dampingC;
	}

	bool isFast = false;
	if (p_keyboardManager->is_key_pressed(KEY_Z))
		isFast = true;
	if (isFast)
		accel *= m_fastCoef;

	moveSpeed_ += accel * m_acceleration * deltaTime;
	const float maxSpeed = isFast ? m_maxSpeed * m_fastCoef : m_maxSpeed;
	if (moveSpeed_.length() > maxSpeed)
	{
		moveSpeed_ = glm::normalize(moveSpeed_);
		moveSpeed_ *= maxSpeed;
	}


	cameraPosition_ += moveSpeed_ * deltaTime;

	//X Build the view matr
	const glm::mat4 t = glm::translate(glm::mat4(1.0f), -cameraPosition_);
	const glm::mat4 r = glm::mat4_cast(cameraOrientation_);
	m_view = r * t;
	m_viewProj = m_projection * m_view;

}

const float* GEditorFPSCameraPositioner::get_position()
{
	return glm::value_ptr(cameraPosition_);
}

bool GEditorFPSCameraPositioner::init()
{
	auto window = ((GSharedPtr<Window>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_WINDOW))->get();
	p_keyboardManager = window->get_keyboard_manager();
	p_mouseManager = window->get_mouse_manager();
	return true;
}

const CameraData* GEditorFPSCameraPositioner::get_camera_data() noexcept 
{
	 return &m_cameraData;
}
void GEditorFPSCameraPositioner::setup_up_vector()
{
	const glm::mat4 t = glm::translate(glm::mat4(1.0f), -cameraPosition_);
	const glm::mat4 r = glm::mat4_cast(cameraOrientation_);
	const glm::mat4 view = r*t;
	const glm::vec3 dir = -glm::vec3(view[0][2], view[1][2], view[2][2]);
	cameraOrientation_ = glm::lookAt(cameraPosition_, cameraPosition_ + dir, up_);
}
