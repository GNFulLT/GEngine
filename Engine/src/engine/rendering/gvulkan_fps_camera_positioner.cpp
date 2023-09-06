#include "internal/engine/rendering/gvulkan_fps_camera_positioner.h"
#include "public/math/gcam.h"
#include "public/math/gtransform.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "public/core/templates/shared_ptr.h"
#include "public/platform/window.h"
#include "engine/manager/iglogger_manager.h"
#include "public/platform/ikeyboard_manager.h"


GFpsCameraPositioner::GFpsCameraPositioner()
{
	m_projMatrix = perspective(70.f, 16.f / 9.f, 0.1f, 1000.f);
	m_viewMatrix = translate(m_campos);
	m_viewProjMatrix = m_projMatrix * m_viewMatrix;
	p_keyboardManager = nullptr;


}

void GFpsCameraPositioner::update(float deltaTime)
{
	if (p_keyboardManager->is_key_pressed(KEY_W))
	{
		m_campos.z += 1.f * deltaTime;
		((GSharedPtr<IGLoggerManager>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_LOGGER))->get()->log_i("GFPSCameraPositioner", "PRESSSING W");
	}
	if (p_keyboardManager->is_key_pressed(KEY_S))
	{
		m_campos.z -= 1.f * deltaTime;
	}
	m_viewMatrix = translate(m_campos);
	m_viewProjMatrix = m_projMatrix * m_viewMatrix;
}

const gmat4* GFpsCameraPositioner::get_view_proj_projection()
{
	
	return &m_viewProjMatrix;
}

const gvec3* GFpsCameraPositioner::get_position()
{
	return nullptr;
}

bool GFpsCameraPositioner::init()
{
	p_keyboardManager = ((GSharedPtr<Window>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_WINDOW))->get()->get_keyboard_manager();
	return true;
}

