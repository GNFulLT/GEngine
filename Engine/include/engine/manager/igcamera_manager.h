#ifndef IGCAMERA_MANAGER_H
#define IGCAMERA_MANAGER_H

#include "engine/GEngine_EXPORT.h"
#include <cstdint>
class ENGINE_API IGCameraManager
{
public:
	virtual ~IGCameraManager() = default;

	virtual bool init() = 0;

	virtual void destroy() = 0;

	virtual void update(float deltaTime) = 0;

	virtual void render(uint32_t frame) = 0;

private:
};
#endif // IGCAMERA_MANAGER_H