#include "internal/engine/rendering/gvulkan_fps_camera_positioner.h"
#include "public/math/gcam.h"
#include "public/math/gtransform.h"

GFpsCameraPositioner::GFpsCameraPositioner()
{
	m_projMatrix = perspective(70.f, 16.f / 9.f, 0.1f, 1000.f);
	auto gcamPos = gvec3(1.f, 0.f, -5.f);
	m_viewMatrix = translate(gcamPos);
	m_viewProjMatrix = m_projMatrix * m_viewMatrix;
}

void GFpsCameraPositioner::update(float deltaTime)
{
}

const gmat4* GFpsCameraPositioner::get_view_proj_projection()
{
	
	return &m_viewProjMatrix;
}

const gvec3* GFpsCameraPositioner::get_position()
{
	return nullptr;
}

