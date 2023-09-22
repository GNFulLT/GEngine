#ifndef IGCAMERA_MANAGER_H
#define IGCAMERA_MANAGER_H

#include "engine/GEngine_EXPORT.h"
#include <cstdint>

class ICameraPositioner;
class IGVulkanUniformBuffer;

#include <glm/glm.hpp>

struct DrawCullData
{
	glm::vec4 frustumPlanes[6];
	glm::vec4 frustumCorners[8];
	float lodBase, lodStep;
	uint32_t drawCount;
};


class ENGINE_API IGCameraManager
{
public:
	virtual ~IGCameraManager() = default;

	virtual bool init() = 0;

	virtual void destroy() = 0;

	virtual void update(float deltaTime) = 0;

	virtual void set_positioner(ICameraPositioner* positioner) = 0;

	virtual const float* get_camera_position() = 0;

	virtual const float* get_camera_view_matrix() = 0;

	virtual const float* get_camera_proj_matrix() = 0;

	virtual const float* get_camera_view_proj_matrix() = 0;


	virtual const DrawCullData* get_cull_data() const noexcept = 0;

	virtual DrawCullData* get_cull_data() noexcept = 0;

	virtual void update_frustrum_proj_matrix(const glm::mat4& frustrum) noexcept = 0;
private:
};
#endif // IGCAMERA_MANAGER_H