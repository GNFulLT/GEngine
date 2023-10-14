#ifndef ICAMERA_POSITIONER_H
#define ICAMERA_POSITIONER_H

#include "engine/GEngine_EXPORT.h"

struct gmat4;
struct gvec3;
struct CameraData
{
	float zNear;
	float zFar;
};

class ICameraPositioner
{
public:
	virtual ~ICameraPositioner() = default;

	virtual void update(float deltaTime) = 0;
	
	virtual const float* get_position() = 0;

	virtual const float* get_view_proj_matrix() const noexcept = 0;

	virtual const float* get_view_matrix() const noexcept = 0;

	virtual const float* get_proj_matrix() const noexcept = 0;
	virtual const CameraData* get_camera_data() noexcept = 0;


	virtual bool init() { return true; };
private:
};

#endif // ICAMERA_POSITIONER_H