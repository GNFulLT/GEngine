#ifndef GVULKAN_FPS_CAMERA_POSITIONER_H
#define GVULKAN_FPS_CAMERA_POSITIONER_H

#include "engine/rendering/icamera_positioner.h"
#include "public/math/gmat4.h"
#include "public/math/gquat.h"

class IKeyboardManager;

class GFpsCameraPositioner : public ICameraPositioner
{
public:
	GFpsCameraPositioner();

	virtual void update(float deltaTime) override;

	virtual const float* get_position() override;

	virtual bool init() override;

	virtual const float* get_view_proj_matrix() const noexcept override;

	virtual const float* get_view_matrix() const noexcept override;

	virtual const float* get_proj_matrix() const noexcept override;

private:

	gmat4 m_viewProjMatrix;

	gmat4 m_viewMatrix;

	gmat4 m_projMatrix;

	gvec3 m_campos = gvec3(1.f, 0.f, -5.f);
	gvec3 m_camDir;
	gvec3 m_camUp = gvec3(0.f, 1.f, 0.f);
	gquat m_camOrientation;

	gvec3 m_camSpeed;

	float m_damping = 0.1f;
	float m_fastCoef = 2.f;
	float m_acceleration = 150.f;
	float m_maxSpeed = 10.0f;

	IKeyboardManager* p_keyboardManager;
};

#endif // GVULKAN_FPS_CAMERA_CONTROLLER_H