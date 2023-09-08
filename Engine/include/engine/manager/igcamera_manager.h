#ifndef IGCAMERA_MANAGER_H
#define IGCAMERA_MANAGER_H

#include "engine/GEngine_EXPORT.h"
#include <cstdint>

class ICameraPositioner;
class IGVulkanUniformBuffer;

class ENGINE_API IGCameraManager
{
public:
	virtual ~IGCameraManager() = default;

	virtual bool init() = 0;

	virtual void destroy() = 0;

	virtual void update(float deltaTime) = 0;

	virtual void render(uint32_t frame) = 0;

	virtual void set_positioner(ICameraPositioner* positioner) = 0;

	virtual IGVulkanUniformBuffer* get_camera_buffer_for_frame(uint32_t frame) = 0;

	virtual uint32_t get_camera_buffer_count() = 0;

	virtual const float* get_camera_position() = 0;
private:
};
#endif // IGCAMERA_MANAGER_H