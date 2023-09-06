#ifndef GVULKAN_FPS_CAMERA_POSITIONER_H
#define GVULKAN_FPS_CAMERA_POSITIONER_H

#include "engine/rendering/icamera_positioner.h"
#include "public/math/gmat4.h"

class IKeyboardManager;

class GFpsCameraPositioner : public ICameraPositioner
{
public:
	GFpsCameraPositioner();

	virtual void update(float deltaTime) override;

	virtual const gmat4* get_view_proj_projection() override;

	virtual const gvec3* get_position() override;

	virtual bool init() override;
private:

	gmat4 m_viewProjMatrix;

	gmat4 m_viewMatrix;

	gmat4 m_projMatrix;

	gvec3 m_campos = gvec3(1.f, 0.f, -5.f);

	IKeyboardManager* p_keyboardManager;
};

#endif // GVULKAN_FPS_CAMERA_CONTROLLER_H