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

	const std::vector<IGVulkanUniformBuffer*>* get_buffers();

	virtual void render(uint32_t frame) override;

	virtual void set_positioner(ICameraPositioner* positioner) override;

	inline static GCameraManager* s_static;
private:
	ICameraPositioner* m_selectedPositioner;

	ICameraPositioner* m_defaultPositioner;

	IGVulkanLogicalDevice* m_boundedDevice;

	uint32_t m_framesInFlight;

	std::vector<IGVulkanUniformBuffer*> m_uniformBuffers;

};


#endif // GCAMERA_MANAGER_H