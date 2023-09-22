#ifndef ICAMERA_POSITIONER_H
#define ICAMERA_POSITIONER_H

#include "engine/GEngine_EXPORT.h"

struct gmat4;
struct gvec3;

class ICameraPositioner
{
public:
	virtual ~ICameraPositioner() = default;

	virtual void update(float deltaTime) = 0;
	
	virtual const float* get_position() = 0;

	virtual const float* get_view_proj_matrix() const noexcept = 0;

	virtual const float* get_view_matrix() const noexcept = 0;

	virtual const float* get_proj_matrix() const noexcept = 0;


	virtual bool init() { return true; };
private:
};

#endif // ICAMERA_POSITIONER_H