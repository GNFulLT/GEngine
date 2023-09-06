#include "internal/engine/rendering/gvulkan_fps_camera_positioner.h"
#include "public/math/gcam.h"
#include "public/math/gtransform.h"

GFpsCameraPositioner::GFpsCameraPositioner()
{
	m_projMatrix = perspective(70.f, 640 / 360, 0.1f, 1000.f);
	auto gcamPos = gvec3(1.f, 0.f, 0.f);
	m_viewMatrix = translate(gcamPos);
	gtransform m_modelTransform;
	auto modelMatrix = m_modelTransform.to_mat4();
	m_viewProjMatrix = m_projMatrix * m_viewMatrix * modelMatrix;
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

