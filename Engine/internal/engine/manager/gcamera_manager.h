#ifndef GCAMERA_MANAGER_H
#define GCAMERA_MANAGER_H

#include "engine/rendering/icamera_positioner.h"


class GCameraManager
{
public:
	GCameraManager();

	bool init();

	void destroy();

	void update(float deltaTime);

	ICameraPositioner* get_current_positioner();
private:
	ICameraPositioner* m_selectedPositioner;

	ICameraPositioner* m_defaultPositioner;

};


#endif // GCAMERA_MANAGER_H