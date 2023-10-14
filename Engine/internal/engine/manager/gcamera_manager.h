#ifndef GCAMERA_MANAGER_H
#define GCAMERA_MANAGER_H

#include "engine/rendering/icamera_positioner.h"
#include "engine/manager/igcamera_manager.h"
#include "public/math/gmat4.h" 
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"
#include <vector>

class GCameraManager :public IGCameraManager
{
public:
	GCameraManager(uint32_t framesInFlight);

	virtual bool init() override;

	virtual void destroy() override;

	virtual void update(float deltaTime) override;

	ICameraPositioner* get_current_positioner();

	virtual void set_positioner(ICameraPositioner* positioner) override;

	virtual const float* get_camera_position() override;

	inline static GCameraManager* s_static;
private:
	ICameraPositioner* m_selectedPositioner;

	ICameraPositioner* m_defaultPositioner;

	IGVulkanLogicalDevice* m_boundedDevice;

	uint32_t m_framesInFlight;

	std::vector<IGVulkanUniformBuffer*> m_uniformBuffers;

	// Inherited via IGCameraManager
	virtual const float* get_camera_view_matrix() override;

	virtual const float* get_camera_proj_matrix() override;

	virtual const float* get_camera_view_proj_matrix() override;

	DrawCullData m_cullData;
	// Inherited via IGCameraManager
	virtual const DrawCullData* get_cull_data() const noexcept override;

	virtual void update_frustrum_proj_matrix(const glm::mat4& frustrum) noexcept override;

	glm::mat4 m_frustrumProjMatrix;

	// Inherited via IGCameraManager
	virtual DrawCullData* get_cull_data() noexcept override;

	// Inherited via IGCameraManager
	virtual const CameraData* get_camera_data() noexcept override;
};


#endif // GCAMERA_MANAGER_H